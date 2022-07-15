/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_static_bvh_cache_component.hpp"
#include "pragma/entities/components/base_static_bvh_user_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"

using namespace pragma;


class FunctionalParallelWorker
	: public util::ParallelWorker<void>
{
public:
	using Task = std::function<void(FunctionalParallelWorker&)>;
	FunctionalParallelWorker();
	void WaitForTask();
	virtual void GetResult() override;

	void ResetTask(const Task &task);
	void CancelTask();
	bool IsTaskCancelled() const;

	using util::ParallelWorker<void>::SetResultMessage;
	using util::ParallelWorker<void>::UpdateProgress;
private:
	using util::ParallelWorker<void>::AddThread;
	virtual void DoCancel(const std::string &resultMsg,std::optional<int32_t> resultCode) override;
	std::mutex m_taskMutex;
	
	Task m_nextTask;
	std::atomic<bool> m_taskCancelled = false;

	std::condition_variable m_taskAvailableCond;

	std::condition_variable m_taskCompleteCond;
	std::mutex m_taskCompleteMutex;

	std::atomic<bool> m_taskAvailable = false;
	std::atomic<bool> m_taskComplete = false;
	std::mutex m_taskAvailableMutex;
	template<typename TJob,typename... TARGS>
		friend util::ParallelJob<typename TJob::RESULT_TYPE> util::create_parallel_job(TARGS&& ...args);
};

BaseStaticBvhCacheComponent::BaseStaticBvhCacheComponent(BaseEntity &ent)
	: BaseBvhComponent(ent)
{}
BaseStaticBvhCacheComponent::~BaseStaticBvhCacheComponent()
{
	if(m_buildWorker)
	{
		m_buildWorker->Cancel();
		m_buildWorker->Wait();
		m_buildWorker = nullptr;
	}
}
void BaseStaticBvhCacheComponent::Initialize()
{
	BaseBvhComponent::Initialize();
}

void BaseStaticBvhCacheComponent::OnRemove()
{
	BaseBvhComponent::OnRemove();
	for(auto *ent :m_entities)
		ent->SetStaticBvhCacheComponent(nullptr);
}

bool FunctionalParallelWorker::IsTaskCancelled() const {return m_taskCancelled;}

void FunctionalParallelWorker::CancelTask()
{
	m_taskMutex.lock();
		m_taskCancelled = true;
		m_taskComplete = false;
		m_nextTask = nullptr;
	m_taskMutex.unlock();
}

void FunctionalParallelWorker::ResetTask(const Task &task)
{
	m_taskMutex.lock();
		m_taskCancelled = true;
		m_taskComplete = false;
		m_nextTask = task;
	m_taskMutex.unlock();

	m_taskAvailableMutex.lock();
		m_taskAvailable = true;
		m_taskAvailableCond.notify_one();
	m_taskAvailableMutex.unlock();
}

FunctionalParallelWorker::FunctionalParallelWorker()
	: util::ParallelWorker<void>{}
{
	AddThread([this]() {
		while(!IsCancelled())
		{
			auto ul = std::unique_lock<std::mutex>{m_taskAvailableMutex};
			m_taskAvailableCond.wait(ul,[this]() -> bool {
				return m_taskAvailable || IsCancelled();
			});
			if(IsCancelled())
				break;
			m_taskMutex.lock();
				auto task = std::move(m_nextTask);
				m_nextTask = nullptr;
				m_taskAvailable = false;
				m_taskCancelled = false;
			m_taskMutex.unlock();

			task(*this);
			if(!IsTaskCancelled())
			{
				m_taskCompleteMutex.lock();
					m_taskComplete = true;
					m_taskCompleteCond.notify_one();
				m_taskCompleteMutex.unlock();
			}
		}
		m_taskComplete = true;
	});
}
void FunctionalParallelWorker::WaitForTask()
{
	auto ul = std::unique_lock<std::mutex>{m_taskCompleteMutex};
	m_taskCompleteCond.wait(ul,[this]() -> bool {
		return m_taskComplete;
	});
}
void FunctionalParallelWorker::GetResult() {return ;}//std::move(m_result);}
void FunctionalParallelWorker::DoCancel(const std::string &resultMsg,std::optional<int32_t> resultCode)
{
	util::ParallelWorker<void>::DoCancel(resultMsg,resultCode);

	m_taskAvailableMutex.lock();
		m_taskAvailable = true;
		m_taskAvailableCond.notify_one();
	m_taskAvailableMutex.unlock();
}

bool BaseStaticBvhCacheComponent::IntersectionTest(
	const Vector3 &origin,const Vector3 &dir,float minDist,float maxDist,
	BvhHitInfo &outHitInfo
) const
{
	const_cast<BaseStaticBvhCacheComponent*>(this)->UpdateBuild();
	if(m_buildWorker)
		m_buildWorker->WaitForTask();
	m_bvhMutex.lock();
		auto res = BaseBvhComponent::IntersectionTest(origin,dir,minDist,maxDist,outHitInfo);
	m_bvhMutex.unlock();
	return res;
}

void BaseStaticBvhCacheComponent::Build(
	std::vector<std::shared_ptr<ModelSubMesh>> &&meshes,
	std::vector<BaseEntity*> &&meshToEntity,
	std::vector<umath::ScaledTransform> &&meshPoses
)
{
	if(!m_buildWorker)
	{
		m_buildWorker = std::make_unique<FunctionalParallelWorker>();
		m_buildWorker->Start();
	}
	m_bvhMutex.lock();
		m_bvhData = nullptr; // No longer valid
	m_bvhMutex.unlock();
	m_buildWorker->ResetTask([this,meshes=std::move(meshes),meshPoses=std::move(meshPoses),meshToEntity=std::move(meshToEntity)]
		(FunctionalParallelWorker &worker) {
		std::vector<size_t> meshIndices;
		auto bvhData = BaseBvhComponent::RebuildBvh(meshes,&meshPoses,[this]() -> bool {
			return m_buildWorker->IsTaskCancelled();
		},&meshIndices);
		if(!bvhData)
			return;
		if(worker.IsTaskCancelled())
			return;
		auto &meshRanges = get_bvh_mesh_ranges(*bvhData);
		for(uint32_t i=0;auto &range : meshRanges)
		{
			range.entity = meshToEntity[meshIndices[i++]];
			if(worker.IsTaskCancelled())
				return;
		}
		if(worker.IsTaskCancelled())
			return;
		m_bvhMutex.lock();
			m_bvhData = std::move(bvhData);
		m_bvhMutex.unlock();
	});
}

void BaseStaticBvhCacheComponent::SetCacheDirty()
{
	m_staticBvhDirty = true;
	SetTickPolicy(TickPolicy::Always);
	
	if(m_buildWorker)
		m_buildWorker->CancelTask();
	m_bvhMutex.lock();
		m_bvhData = nullptr;
	m_bvhMutex.unlock();
}
void BaseStaticBvhCacheComponent::OnTick(double tDelta)
{
	SetTickPolicy(TickPolicy::Never);
	UpdateBuild();
}
void BaseStaticBvhCacheComponent::UpdateBuild()
{
	if(!m_staticBvhDirty)
		return;
	m_staticBvhDirty = false;
	TestRebuildBvh();
}
void BaseStaticBvhCacheComponent::SetEntityDirty(BaseEntity &ent)
{
	// TODO: Only reload meshes for this entity (unless meshes themselves have changed)
	SetCacheDirty();

}
void BaseStaticBvhCacheComponent::AddEntity(BaseEntity &ent)
{
	auto *c = static_cast<BaseStaticBvhUserComponent*>(ent.AddComponent("static_bvh_user").get());
	auto it = m_entities.find(c);
	if(it != m_entities.end())
		return;
	c->SetStaticBvhCacheComponent(this);
	m_entities.insert(c);
	
	SetCacheDirty();
	c->UpdateBvhStatus();
}
void BaseStaticBvhCacheComponent::RemoveEntity(BaseEntity &ent,bool removeFinal)
{
	auto *c = static_cast<BaseStaticBvhUserComponent*>(ent.FindComponent("static_bvh_user").get());
	if(!c)
		return;
	if(removeFinal)
		c->SetStaticBvhCacheComponent(nullptr);
	auto it = m_entities.find(c);
	if(it == m_entities.end())
		return;
	SetCacheDirty();
	m_entities.erase(it);
}
