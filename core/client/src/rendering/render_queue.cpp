/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

using namespace pragma::rendering;

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
	distance = glm::floatBitsToUint(static_cast<float>(distSqr));
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
void RenderQueue::Add(CBaseEntity &ent,RenderMeshIndex meshIdx,CMaterial &mat,pragma::ShaderTextured3DBase &shader,const CCameraComponent *optCam)
{
	Reserve();
	queue.push_back({});
	auto &item = queue.back();
	item.material = mat.GetIndex();
	item.shader = shader.GetIndex();
	item.entity = ent.GetLocalIndex();
	item.mesh = meshIdx;
	item.sortingKey.material = item.material;
	item.sortingKey.shader = item.shader;
	if(optCam)
	{
		// TODO: This isn't very efficient, find a better way to handle this!
		auto *renderC = ent.GetRenderComponent();
		if(renderC)
		{
			auto &renderMeshes = renderC->GetRenderMeshes();
			if(meshIdx < renderMeshes.size())
			{
				auto &pose = ent.GetPose();
				auto pos = pose *renderMeshes.at(meshIdx)->GetCenter();
				item.sortingKey.SetDistance(pos,*optCam);
			}
		}
	}
	sortedItemIndices.push_back({queue.size() -1,item.sortingKey});
}
void RenderQueue::Add(const RenderQueueItem &item)
{
	Reserve();
	queue.push_back(item);
	sortedItemIndices.push_back({queue.size() -1,item.sortingKey});
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
void RenderQueue::WaitForCompletion() const
{
	std::unique_lock<std::mutex> mlock(m_threadWaitMutex);
	m_threadWaitCondition.wait(mlock,[this]() -> bool {return !m_locked;});
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
	if(m_thread.joinable())
		m_thread.join();
}

void RenderQueueBuilder::Exec()
{
	m_threadRunning = true;
	m_thread = std::thread{[this]() {
		std::unique_lock<std::mutex> mlock(m_threadWaitMutex);
		for(;;)
		{
			m_threadWaitCondition.wait(mlock,[this]() -> bool {return m_threadRunning || m_hasWork;});
			m_workMutex.lock();
				if(m_workQueue.empty())
				{
					if(m_threadRunning == false)
						break;
					m_hasWork = false;
					m_workMutex.unlock();
					continue;
				}
				auto worker = m_workQueue.front();
				m_workQueue.pop();
			m_workMutex.unlock();

			worker();
		}
	}};
}

void RenderQueueBuilder::Append(const std::function<void()> &worker)
{
	m_workMutex.lock();
		m_workQueue.push(worker);
		m_hasWork = true;
	m_workMutex.unlock();
	m_threadWaitCondition.notify_one();
}

void RenderQueueBuilder::Flush()
{
	// TODO: Use a condition variable?
	while(m_hasWork);
}

void RenderQueueBuilder::BuildRenderQueues()
{

}
