/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/render_stats.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include <cmaterial.h>

using namespace pragma::rendering;

SortingKey::SortingKey(MaterialIndex material,prosper::ShaderIndex shader,bool instantiable,bool translucentKey)
{
	if(translucentKey)
	{
		translucent.material = material;
		translucent.shader = shader;
		translucent.instantiable = instantiable;
	}
	else
	{
		opaque.material = material;
		opaque.shader = shader;
		opaque.instantiable = instantiable;
	}
}
void SortingKey::SetDistance(const Vector3 &origin,const CCameraComponent &cam)
{
	auto &p0 = origin;
	auto &p1 = cam.GetEntity().GetPosition();
	auto v = p1 -p0;
	// Double precision since we're working with the square of the far plane
	auto distSqr = umath::pow2(static_cast<double>(v.x)) +umath::pow2(static_cast<double>(v.y)) +umath::pow2(static_cast<double>(v.z));
	auto nearZ = umath::pow2(static_cast<double>(cam.GetNearZ()));
	auto farZ = umath::pow2(static_cast<double>(cam.GetFarZ()));
	// Map the distance to [0,1] and invert (since we want objects that are furthest away to be rendered first)
	distSqr = 1.0 -std::clamp(distSqr /(farZ +nearZ),0.0,1.0);
	// Note: 16 bit precision is not enough, but 24 bit might be. For now we'll use a 32 bit integer,
	// since we have that much space available anyway
	translucent.distance = glm::floatBitsToUint(static_cast<float>(distSqr));
}

RenderQueueItem::RenderQueueItem(CBaseEntity &ent,RenderMeshIndex meshIdx,CMaterial &mat,prosper::PipelineID pipelineId,const CCameraComponent *optCam)
	: material{mat.GetIndex()},pipelineId{pipelineId},entity{ent.GetLocalIndex()},mesh{meshIdx},translucentKey{optCam ? true : false}
{
	instanceSetIndex = RenderQueueItem::UNIQUE;
	auto &renderC = *ent.GetRenderComponent();
	auto instantiable = renderC.IsInstantiable();
	if(optCam)
	{
		// TODO: This isn't very efficient, find a better way to handle this!
		auto &renderMeshes = renderC.GetRenderMeshes();
		if(meshIdx < renderMeshes.size())
		{
			auto &pose = ent.GetPose();
			auto pos = pose *renderMeshes[meshIdx]->GetCenter();
			sortingKey.SetDistance(pos,*optCam);
		}

		sortingKey.translucent.material = material;
		sortingKey.translucent.shader = pipelineId;
		sortingKey.translucent.instantiable = renderC.IsInstantiable();
	}
	else
	{
		sortingKey.opaque.distance = 0;
		sortingKey.opaque.material = material;
		sortingKey.opaque.shader = pipelineId;
		sortingKey.opaque.instantiable = renderC.IsInstantiable();
	}
}

std::shared_ptr<RenderQueue> RenderQueue::Create() {return std::shared_ptr<RenderQueue>{new RenderQueue{}};}

RenderQueue::RenderQueue()
{}

void RenderQueue::Reserve()
{
	if(queue.size() < queue.capacity())
		return;
	auto newCapacity = queue.size() *1.1 +100;
	queue.reserve(newCapacity);
	sortedItemIndices.reserve(newCapacity);
}
void RenderQueue::Clear()
{
	queue.clear();
	sortedItemIndices.clear();
}
void RenderQueue::Add(CBaseEntity &ent,RenderMeshIndex meshIdx,CMaterial &mat,prosper::PipelineID pipelineId,const CCameraComponent *optCam)
{
	Add({ent,meshIdx,mat,pipelineId,optCam});
}
void RenderQueue::Add(const RenderQueueItem &item)
{
	m_queueMutex.lock();
		Reserve();
		queue.push_back(item);
		sortedItemIndices.push_back({queue.size() -1,item.sortingKey});
	m_queueMutex.unlock();
}
void RenderQueue::Add(const std::vector<RenderQueueItem> &items)
{
	m_queueMutex.lock();
		auto offset = queue.size();
		queue.resize(queue.size() +items.size());
		sortedItemIndices.resize(queue.size());
		for(auto i=decltype(items.size()){0u};i<items.size();++i)
		{
			queue[offset +i] = items[i];
			sortedItemIndices[offset +i] = {offset +i,items[i].sortingKey};
		}
	m_queueMutex.unlock();
}
void RenderQueue::Sort()
{
	std::sort(sortedItemIndices.begin(),sortedItemIndices.end(),[](const RenderQueueItemSortPair &a,const RenderQueueItemSortPair &b) {
		static_assert(sizeof(decltype(a.second)) == sizeof(uint64_t));
		return *reinterpret_cast<const uint64_t*>(&a.second) < *reinterpret_cast<const uint64_t*>(&b.second);
	});
}

void RenderQueue::Merge(const RenderQueue &other)
{
	queue.reserve(queue.size() +other.queue.size());
	sortedItemIndices.reserve(queue.size());
	for(auto i=decltype(other.queue.size()){0u};i<other.queue.size();++i)
	{
		auto &item = other.queue.at(i);
		queue.push_back(item);
		sortedItemIndices.push_back(other.sortedItemIndices.at(i));
		sortedItemIndices.back().first = queue.size() -1;
	}
}

void RenderQueue::Lock()
{
	m_threadWaitMutex.lock();
		m_locked = true;
	m_threadWaitMutex.unlock();
}
void RenderQueue::Unlock()
{
	m_threadWaitMutex.lock();
		m_locked = false;
		m_threadWaitCondition.notify_all();
	m_threadWaitMutex.unlock();
}
bool RenderQueue::IsComplete() const {return !m_locked;}
void RenderQueue::WaitForCompletion(RenderPassStats *optStats) const
{
	std::chrono::steady_clock::time_point t;
	if(optStats)
		t = std::chrono::steady_clock::now();

	std::unique_lock<std::mutex> mlock(m_threadWaitMutex);
	m_threadWaitCondition.wait(mlock,[this]() -> bool {return !m_locked;});

	if(optStats)
		(*optStats)->AddTime(RenderPassStats::Timer::RenderThreadWait,std::chrono::steady_clock::now() -t);
}

//////////////////////

RenderQueueBuilder::RenderQueueBuilder()
{
	Exec();
}

RenderQueueBuilder::~RenderQueueBuilder()
{
	m_threadRunning = false;
	Flush();
	m_threadWaitCondition.notify_one();
	if(m_thread.joinable())
		m_thread.join();
}

void RenderQueueBuilder::Exec()
{
	m_threadRunning = true;
	m_thread = std::thread{[this]() {
		for(;;)
		{
			std::unique_lock<std::mutex> mlock {m_workMutex};
			m_threadWaitCondition.wait(mlock,[this]() -> bool {return !m_threadRunning || m_hasWork;});
			
			if(m_workQueue.empty())
			{
				m_hasWork = false;
				if(m_threadRunning == false)
					return;
				continue;
			}
			auto worker = m_workQueue.front();
			m_workQueue.pop();

			mlock.unlock();
			worker();
		}
	}};
}

void RenderQueueBuilder::Append(const std::function<void()> &worker)
{
	m_workMutex.lock();
		m_workQueue.push(worker);
		m_hasWork = true;
		m_threadWaitCondition.notify_one();
	m_workMutex.unlock();
}

void RenderQueueBuilder::Flush()
{
	// TODO: Use a condition variable?
	while(m_hasWork);
}

bool RenderQueueBuilder::HasWork() const {return m_hasWork;}
uint32_t RenderQueueBuilder::GetWorkQueueCount() const
{
	const_cast<RenderQueueBuilder*>(this)->m_workMutex.lock();
		auto numEls = m_workQueue.size();
	const_cast<RenderQueueBuilder*>(this)->m_workMutex.unlock();
	return numEls;
}

void RenderQueueBuilder::BuildRenderQueues()
{

}
