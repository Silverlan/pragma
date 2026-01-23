// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.animated_bvh;
import :entities.components.model;
import :engine;

using namespace pragma;

static std::unique_ptr<ThreadPool> g_threadPool = nullptr;
static uint32_t g_instanceCount = 0;

static ThreadPool &get_thread_pool() { return *g_threadPool; }
static void init_thread_pool() { g_threadPool = std::make_unique<ThreadPool>(8, "bvh_animated"); }
static void free_thread_pool() { g_threadPool = nullptr; }

CAnimatedBvhComponent::CAnimatedBvhComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent)
{
	if(g_instanceCount++ == 0)
		init_thread_pool();
}
void CAnimatedBvhComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CAnimatedBvhComponent::UpdateDirtyBones()
{
	if(IsBusy())
		return;
	auto animC = GetEntity().GetComponent<CAnimatedComponent>();
	auto &processedPoses = animC->GetProcessedBones();
	if(m_prevBonePoses.size() != processedPoses.size()) {
		m_prevBonePoses.resize(processedPoses.size());
		for(auto i = decltype(processedPoses.size()) {0u}; i < processedPoses.size(); ++i) {
			m_prevBonePoses[i] = processedPoses[i].GetOrigin();
		}

		// Number of bones has changed, need to do a full rebuild!
		RebuildAnimatedBvh();
		return;
	}

	constexpr auto thresholdDistance = math::pow2(0.4f);
	m_dirtyBones.clear();
	m_dirtyBones.resize(processedPoses.size(), false);
	auto hasDirtyBones = false;
	for(auto i = decltype(processedPoses.size()) {0u}; i < processedPoses.size(); ++i) {
		auto &oldPose = m_prevBonePoses[i];
		auto &newPose = processedPoses[i];
		auto d = uquat::dot_product(oldPose.GetRotation(), newPose.GetRotation());
		if(uvec::distance_sqr(newPose.GetOrigin(), oldPose.GetOrigin()) >= thresholdDistance || d < 0.999f) {
			m_dirtyBones[i] = true;
			hasDirtyBones = true;
			oldPose = newPose;
		}
	}

	if(!hasDirtyBones)
		return;

	// Update vertices associated with dirty bones only
	RebuildAnimatedBvh(false, &m_dirtyBones);
}

void CAnimatedBvhComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto animC = GetEntity().GetComponent<CAnimatedComponent>();
	if(animC.valid()) {
		m_cbOnMatricesUpdated = animC->AddEventCallback(cAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
			UpdateDirtyBones();
			return util::EventReply::Unhandled;
		});
		animC->SetSkeletonUpdateCallbacksEnabled(true);
	}

	auto bvhC = GetEntity().GetComponent<CBvhComponent>();
	if(bvhC.valid()) {
		m_cbOnBvhCleared = bvhC->AddEventCallback(cBvhComponent::EVENT_ON_CLEAR_BVH, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
			Clear();
			m_tmpBvhData = nullptr;
			return util::EventReply::Unhandled;
		});
		m_cbOnBvhUpdateRequested = bvhC->AddEventCallback(cBvhComponent::EVENT_ON_BVH_UPDATE_REQUESTED, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
			/*if(m_updateLazily && m_rebuildScheduled)
			{
				RebuildAnimatedBvh(true);
				WaitForCompletion();
			}*/
			return util::EventReply::Unhandled;
		});
		m_cbOnBvhRebuilt = bvhC->AddEventCallback(cBvhComponent::EVENT_ON_BVH_REBUILT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
			RebuildTemporaryBvhData();
			return util::EventReply::Unhandled;
		});

		if(bvhC->HasBvhData())
			RebuildTemporaryBvhData();
	}
}

void CAnimatedBvhComponent::RebuildTemporaryBvhData()
{
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(!mdlC)
		return;
	auto &renderMeshes = mdlC->GetRenderMeshes();
	Clear();

	CBvhComponent::BvhBuildInfo buildInfo {};
	buildInfo.shouldConsiderMesh = [mdlC](const geometry::ModelSubMesh &mesh, uint32_t meshIdx) -> bool { return CBvhComponent::ShouldConsiderMesh(mesh, *mdlC->GetRenderBufferData(meshIdx)); };
	m_tmpBvhData = BaseBvhComponent::RebuildBvh(renderMeshes, &buildInfo, nullptr, &GetEntity());
}

void CAnimatedBvhComponent::SetUpdateLazily(bool updateLazily) { m_updateLazily = updateLazily; }
bool CAnimatedBvhComponent::ShouldUpdateLazily() const { return m_updateLazily; }

void CAnimatedBvhComponent::Clear()
{
	Cancel();
	WaitForCompletion();
}

void CAnimatedBvhComponent::OnRemove()
{
	Clear();
	m_tmpBvhData = nullptr;

	if(m_cbOnMatricesUpdated.IsValid())
		m_cbOnMatricesUpdated.Remove();
	if(m_cbOnBvhCleared.IsValid())
		m_cbOnBvhCleared.Remove();
	if(m_cbOnBvhUpdateRequested.IsValid())
		m_cbOnBvhUpdateRequested.Remove();
	if(m_cbRebuildScheduled.IsValid())
		m_cbRebuildScheduled.Remove();

	auto animC = GetEntity().GetComponent<CAnimatedComponent>();
	if(animC.valid())
		animC->SetSkeletonUpdateCallbacksEnabled(false);

	BaseEntityComponent::OnRemove();
	if(--g_instanceCount == 0)
		free_thread_pool();
}

void CAnimatedBvhComponent::Cancel() { m_cancelled = true; }
bool CAnimatedBvhComponent::IsBusy() const
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
	std::unique_lock<std::mutex> lock {m_animatedBvhData.completeMutex};
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	return m_animatedBvhData.completeCount != m_numJobs;
}
void CAnimatedBvhComponent::WaitForCompletion()
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
	std::unique_lock<std::mutex> lock {m_animatedBvhData.completeMutex};
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	m_animatedBvhData.completeCondition.wait(lock, [this]() { return m_animatedBvhData.completeCount == m_numJobs; });
}

void CAnimatedBvhComponent::RebuildAnimatedBvh(bool force) { return RebuildAnimatedBvh(force, nullptr); }

void CAnimatedBvhComponent::RebuildAnimatedBvh(bool force, const std::vector<bool> *optDirtyBones)
{
	if(IsBusy()) // TODO: Cancel current rebuild if new rebuild is a *complete* rebuild and old rebuild isn't
		return;
	/*if(!force && m_updateLazily)
	{
		m_rebuildScheduled = true;
		if(m_cbRebuildScheduled.IsValid())
			m_cbRebuildScheduled.Remove();

		auto bvhC = GetEntity().GetComponent<CBvhComponent>();
		if(bvhC.valid())
			bvhC->SendBvhUpdateRequestOnInteraction();
		return;
	}*/
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_animated_prepare");
#endif
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	pragma::util::ScopeGuard sg {[]() { ::debug::get_domain().EndTask(); }};
#endif
	if(!force && IsBusy()) {
		if(m_rebuildScheduled)
			return;
		m_rebuildScheduled = true;
		if(m_cbRebuildScheduled.IsValid())
			m_cbRebuildScheduled.Remove();
		m_cbRebuildScheduled = get_cengine()->AddCallback("Think", FunctionCallback<void>::Create([this]() { RebuildAnimatedBvh(); }));
		return;
	}
	if(m_cbRebuildScheduled.IsValid())
		m_cbRebuildScheduled.Remove();

	Clear();
	m_numJobs = 0;
	m_animatedBvhData.completeCount = 0;
	m_cancelled = false;
	m_rebuildScheduled = false;

	auto *animC = static_cast<CAnimatedComponent *>(GetEntity().GetAnimatedComponent().get());
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(!animC || !mdlC)
		return;

	// Need to copy the current bone matrices
	auto &animBvhData = m_animatedBvhData.animationBvhData;
	animBvhData.boneMatrices = animC->GetBoneMatrices();
	m_animatedBvhData.renderMeshes = mdlC->GetRenderMeshes();
	m_tStart = std::chrono::steady_clock::now();

	auto &renderMeshes = m_animatedBvhData.renderMeshes;
	auto &pool = get_thread_pool();

	// Prepare mesh data
	constexpr uint32_t numVerticesPerBatch = 5'000;
	uint32_t &numJobs = m_numJobs;
	numJobs = 0;
	size_t numIndices = 0;
	for(auto it = renderMeshes.begin(); it != renderMeshes.end();) {
		auto &renderMesh = *it;
		auto idx = it - renderMeshes.begin();
		if(!CBvhComponent::ShouldConsiderMesh(*renderMesh, *mdlC->GetRenderBufferData(idx))) {
			it = renderMeshes.erase(it);
			continue;
		}
		auto numVerts = renderMesh->GetVertexCount();
		numJobs += numVerts / numVerticesPerBatch;
		if((numVerts % numVerticesPerBatch) > 0)
			++numJobs;
		numIndices += renderMesh->GetIndexCount();
		++it;
	}
	if(numIndices == 0)
		return;
	auto bvhC = GetEntity().GetComponent<CBvhComponent>();
	if(bvhC.expired())
		return;
	m_busy = true;

	auto &meshDatas = m_animatedBvhData.meshData;
	meshDatas.resize(renderMeshes.size());

	auto triCount = (numIndices / 3);
	if(triCount != m_animatedBvhData.transformedTris.size()) {
		optDirtyBones = nullptr; // Full update required
		m_animatedBvhData.transformedTris.resize(triCount);
	}

	std::function<bool(uint32_t, const math::Vertex &, const math::VertexWeight &)> fShouldConsiderVertex = nullptr;
	if(optDirtyBones) {
		auto cpyDirtyBones = *optDirtyBones;
		fShouldConsiderVertex = [cpyDirtyBones = std::move(cpyDirtyBones)](uint32_t vertIdx, const math::Vertex &v, const math::VertexWeight &vw) -> bool {
			constexpr auto n = decltype(vw.boneIds)::length();
			for(auto i = decltype(n) {0u}; i < n; ++i) {
				assert(vw.boneIds[i] < cpyDirtyBones.size());
				if(vw.boneIds[i] >= 0 && cpyDirtyBones[vw.boneIds[i]])
					return true;
			}
			return false;
		};
	}

	auto finalize = [this, bvhC, &animBvhData, &renderMeshes]() mutable -> ThreadPool::ResultHandler {
		size_t indexOffset = 0;
		uint32_t meshIdx = 0;
		for(auto &renderMesh : renderMeshes) {
			renderMesh->VisitIndices([this, meshIdx, &indexOffset](auto *indexDataSrc, uint32_t numIndicesSrc) {
				auto &verts = m_animatedBvhData.meshData.at(meshIdx).transformedVerts;
				for(auto i = decltype(numIndicesSrc) {0}; i < numIndicesSrc; i += 3)
					m_animatedBvhData.transformedTris[(indexOffset + i) / 3] = {bvh::create_triangle(verts[indexDataSrc[i]], verts[indexDataSrc[i + 1]], verts[indexDataSrc[i + 2]])};
				indexOffset += numIndicesSrc;
			});
			++meshIdx;
		}

		// Swap Bvh
		CBvhComponent::SetVertexData(*m_tmpBvhData, m_animatedBvhData.transformedTris);
		auto oldBvh = bvhC->SetBvhData(m_tmpBvhData);
		m_tmpBvhData = oldBvh;

		auto dt = std::chrono::steady_clock::now() - m_tStart;
		auto timePassed = dt.count() / 1'000'000.0;
		// std::cout << "Time: " << timePassed << "ms" << std::endl;

		m_busy = false;
		return {};
	};
	for(auto i = decltype(renderMeshes.size()) {0u}; i < renderMeshes.size(); ++i) {
		auto &mesh = *renderMeshes[i];
		auto numVerts = mesh.GetVertexCount();
		auto &meshData = meshDatas[i];
		meshData.transformedVerts.resize(numVerts);

		pool.BatchProcess(numVerts, numVerticesPerBatch, [this, fShouldConsiderVertex, numJobs, &mesh, &meshData, &animBvhData, finalize](uint32_t start, uint32_t end) mutable -> ThreadPool::ResultHandler {
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
			::debug::get_domain().BeginTask("bvh_animated_compute");
#endif
			auto &verts = mesh.GetVertices();
			auto &vertexWeights = mesh.GetVertexWeights();
			auto &transformedVerts = meshData.transformedVerts;
			assert(transformedVerts.size() == vertexWeights.size());
			if(transformedVerts.size() != vertexWeights.size()) {
				for(auto i = start; i < end; ++i)
					transformedVerts[i] = verts[i].position;
			}
			else {
				for(auto i = start; i < end; ++i) {
					if(m_cancelled)
						break;
					auto &v = verts[i];
					auto &vw = vertexWeights[i];
					if(fShouldConsiderVertex && fShouldConsiderVertex(i, v, vw) == false)
						continue;
					Mat4 mat {0.f};
					for(auto i = 0u; i < 4u; ++i) {
						auto boneId = vw.boneIds[i];
						if(boneId == -1)
							continue;
						auto weight = vw.weights[i];
						mat += weight * animBvhData.boneMatrices[boneId];
						// TODO: Include flexes
					}

					Vector4 vpos {v.position.x, v.position.y, v.position.z, 1.f};
					vpos = mat * vpos;
					transformedVerts[i] = Vector3 {vpos.x, vpos.y, vpos.z} / vpos.w;
				}
			}
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
			::debug::get_domain().EndTask();
			::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
			m_animatedBvhData.completeMutex.lock();
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
			::debug::get_domain().EndTask();
#endif
			if(++m_animatedBvhData.completeCount == numJobs) {
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
				::debug::get_domain().BeginTask("bvh_animated_finalize");
#endif
				finalize();
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
				::debug::get_domain().EndTask();
#endif
				m_animatedBvhData.completeCondition.notify_one();
			}
			m_animatedBvhData.completeMutex.unlock();
			return {};
		});
	}
}
