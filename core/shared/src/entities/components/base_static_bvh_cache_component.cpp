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
#include "pragma/util/functional_parallel_worker.hpp"
#include "pragma/logging.hpp"

using namespace pragma;

static spdlog::logger &LOGGER = pragma::register_logger("bvh");

BaseStaticBvhCacheComponent::BaseStaticBvhCacheComponent(BaseEntity &ent) : BaseBvhComponent(ent) {}
BaseStaticBvhCacheComponent::~BaseStaticBvhCacheComponent()
{
	if(m_buildWorker) {
		LOGGER.info("Cancelling BVH cache build...");
		m_buildWorker->Cancel();
		m_buildWorker->Wait();
		m_buildWorker = nullptr;
	}
}
void BaseStaticBvhCacheComponent::Initialize()
{
	BaseBvhComponent::Initialize();

	// Make sure to remove the static bvh cache before everything else when the game shuts down, otherwise the shutdown may take a long time
	m_onEndGame = GetGame().AddCallback("EndGame", FunctionCallback<void>::Create([this]() { GetEntity().Remove(); }));
}

void BaseStaticBvhCacheComponent::OnRemove()
{
	BaseBvhComponent::OnRemove();
	if(m_onEndGame.IsValid())
		m_onEndGame.Remove();
	for(auto *ent : m_entities)
		ent->SetStaticBvhCacheComponent(nullptr);
}

bool BaseStaticBvhCacheComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, BvhHitInfo &outHitInfo) const
{
	//const_cast<BaseStaticBvhCacheComponent *>(this)->UpdateBuild();
	//if(m_buildWorker)
	//	m_buildWorker->WaitForTask();
	return BaseBvhComponent::IntersectionTest(origin, dir, minDist, maxDist, outHitInfo);
}

void BaseStaticBvhCacheComponent::Build(std::vector<std::shared_ptr<ModelSubMesh>> &&meshes, std::vector<BaseEntity *> &&meshToEntity, std::vector<umath::ScaledTransform> &&meshPoses)
{
	LOGGER.info("Building new static BVH cache...");
	m_bvhInitialized = true;
	if(!m_buildWorker) {
		m_buildWorker = std::make_unique<util::FunctionalParallelWorker>(true);
		m_buildWorker->Start();
	}
	// m_bvhDataMutex.lock();
	// m_bvhData = nullptr; // No longer valid
	// m_bvhDataMutex.unlock();
	m_buildWorker->CancelTask();
	m_bvhPendingWorkerResult = std::unique_ptr<BvhPendingWorkerResult> {new BvhPendingWorkerResult {}};
	m_buildWorker->ResetTask([this, meshes = std::move(meshes), meshPoses = std::move(meshPoses), meshToEntity = std::move(meshToEntity)](util::FunctionalParallelWorker &worker) {
		std::vector<size_t> meshIndices;
		BaseBvhComponent::BvhBuildInfo buildInfo {};
		buildInfo.isCancelled = [this]() -> bool { return m_buildWorker->IsTaskCancelled(); };
		buildInfo.poses = &meshPoses;
		auto bvhData = BaseBvhComponent::RebuildBvh(meshes, &buildInfo, &meshIndices);
		if(!bvhData)
			return;
		if(worker.IsTaskCancelled())
			return;
		auto &meshRanges = get_bvh_mesh_ranges(*bvhData);
		for(uint32_t i = 0; auto &range : meshRanges) {
			range.entity = meshToEntity[meshIndices[i++]];
			if(worker.IsTaskCancelled())
				return;
		}
		if(worker.IsTaskCancelled())
			return;
		m_bvhPendingWorkerResult->bvhData = std::move(bvhData);
		m_bvhPendingWorkerResult->complete = true;
	});

	SetTickPolicy(TickPolicy::Always);
}

void BaseStaticBvhCacheComponent::SetCacheDirty()
{
	if(m_staticBvhDirty)
		return;
	LOGGER.info("Marking static BVH cache as dirty...");
	m_staticBvhDirty = true;
	SetTickPolicy(TickPolicy::Always);
}
void BaseStaticBvhCacheComponent::OnTick(double tDelta)
{
	if(m_bvhPendingWorkerResult) {
		if(m_bvhPendingWorkerResult->complete) {
			auto pendingResult = std::move(m_bvhPendingWorkerResult);
			m_bvhPendingWorkerResult = nullptr;

			LOGGER.info("Finalizing new static BVH cache (version {})...", m_currentBvhCacheVersion);
			m_bvhDataMutex.lock();
			m_bvhData = pendingResult->bvhData;
			m_bvhDataMutex.unlock();

			while(!pendingResult->callOnComplete.empty()) {
				auto &f = pendingResult->callOnComplete.front();
				f();
				pendingResult->callOnComplete.pop();
			}

			++m_currentBvhCacheVersion;
			for(auto *userC : m_entities) {
				if(userC->HasDynamicBvhSubstitute() && userC->GetStaticBvhCacheVersion() <= m_currentBvhCacheVersion) {
					LOGGER.info("Destroying dynamic BVH substitution for entity {}...", userC->GetEntity().ToString());
					userC->DestroyDynamicBvhSubstitute(); // We no longer need the dynamic BVH for this, the entity should be up-to-date with the static BVH
				}
			}
		}
	}
	else {
		SetTickPolicy(TickPolicy::Never);
		UpdateBuild();
	}
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
	SetCacheDirty();

	if(!m_bvhInitialized)
		return;

	// Immediately remove entity from BVH
	RemoveEntityFromBvh(ent);

	auto *c = static_cast<BaseStaticBvhUserComponent *>(ent.AddComponent("static_bvh_user").get());
	if(c) {
		LOGGER.info("Initializing dynamic BVH substitution for entity {} (static BVH cache version {})...", ent.ToString(), m_currentBvhCacheVersion + 1);
		c->InitializeDynamicBvhSubstitute(m_currentBvhCacheVersion + 1); // Temporarily initialize a dynamic BVH until the new static BVH has been built
	}
}
void BaseStaticBvhCacheComponent::AddEntity(BaseEntity &ent)
{
	auto *c = static_cast<BaseStaticBvhUserComponent *>(ent.AddComponent("static_bvh_user").get());
	auto it = m_entities.find(c);
	if(it != m_entities.end())
		return;
	c->SetStaticBvhCacheComponent(this);
	m_entities.insert(c);
	c->UpdateBvhStatus();

	SetEntityDirty(ent);
}
void BaseStaticBvhCacheComponent::RemoveEntity(BaseEntity &ent, bool removeFinal)
{
	auto *c = static_cast<BaseStaticBvhUserComponent *>(ent.FindComponent("static_bvh_user").get());
	if(!c)
		return;
	if(removeFinal)
		c->SetStaticBvhCacheComponent(nullptr);
	auto it = m_entities.find(c);
	if(it == m_entities.end())
		return;
	SetCacheDirty();
	m_entities.erase(it);
	RemoveEntityFromBvh(ent);
}
void BaseStaticBvhCacheComponent::RemoveEntityFromBvh(const BaseEntity &ent)
{
	m_bvhDataMutex.lock();
	if(m_bvhData) {
		LOGGER.info("Removing entity {} from static BVH cache...", ent.ToString());

		// Delete the entity from the current BVH
		auto &meshRanges = get_bvh_mesh_ranges(*m_bvhData);
		auto it = std::find_if(meshRanges.begin(), meshRanges.end(), [&ent](const BvhMeshRange &range) { return range.entity == &ent; });
		if(it != meshRanges.end()) {
			auto &meshRange = *it;
			BaseBvhComponent::DeleteRange(*m_bvhData, meshRange.start, meshRange.end);
		}
	}
	m_bvhDataMutex.unlock();

	if(m_bvhPendingWorkerResult) {
		// A new BVH is currently being built, we'll have to remove the entity once it is complete
		auto hEnt = ent.GetHandle();
		m_bvhPendingWorkerResult->callOnComplete.push([this, hEnt]() {
			if(hEnt.IsValid() == false)
				return;
			RemoveEntityFromBvh(*hEnt.get());
		});
	}
}
