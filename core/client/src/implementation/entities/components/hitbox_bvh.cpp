// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <bvh/v2/default_builder.h>
#include <bvh/v2/stack.h>

module pragma.client;

import :entities.components.hitbox_bvh;
import :engine;
import :debug;
import :entities.components.animated;
import :entities.components.render;

using namespace pragma;

static spdlog::logger &LOGGER = register_logger("bvh");

static std::shared_ptr<pragma::bvh::HitboxBvhCache> g_hbBvhCache {};
static std::unique_ptr<BS::light_thread_pool> g_hbThreadPool {};
static size_t g_hbBvhCount = 0;

pragma::bvh::HitboxBvhCache::HitboxBvhCache(Game &game) : m_game {game} {}
pragma::bvh::HitboxBvhCache::~HitboxBvhCache()
{
	for(auto &[mdlName, mdlCache] : m_modelBvhCache) {
		if(mdlCache->task.valid())
			mdlCache->task.wait();
	}
}
pragma::bvh::ModelHitboxBvhCache *pragma::bvh::HitboxBvhCache::GetModelCache(const ModelName &mdlName)
{
	auto normName = pragma::asset::get_normalized_path(mdlName, asset::Type::Model);
	auto it = m_modelBvhCache.find(normName);
	return (it != m_modelBvhCache.end() && it->second->complete) ? it->second.get() : nullptr;
}
void pragma::bvh::HitboxBvhCache::PrepareModel(asset::Model &mdl)
{
	auto t = std::chrono::steady_clock::now();
	auto savingRequired = false;
	if(mdl.GenerateLowLevelLODs(m_game))
		savingRequired = true;
	if(mdl.GetHitboxCount() == 0 && mdl.GenerateHitboxes())
		savingRequired = true;
	if(savingRequired) {
		std::string err;
		if(!mdl.Save(m_game, err))
			LOGGER.warn("Failed to save model '{}': {}", mdl.GetName(), err);
	}
}
void pragma::bvh::HitboxBvhCache::InitializeModelHitboxBvhCache(asset::Model &mdl, const HitboxMeshBvhBuildTask &buildTask, ModelHitboxBvhCache &mdlHbBvhCache)
{
	auto &skeleton = mdl.GetSkeleton();
	auto &boneCaches = mdlHbBvhCache.boneCache;
	auto &boneMeshMap = buildTask.GetResult();
	for(auto &[boneName, boneMeshInfos] : boneMeshMap) {
		auto boneId = skeleton.LookupBone(boneName);
		if(boneId < 0)
			continue;
		for(auto &boneMeshInfo : boneMeshInfos) {
			auto it = boneCaches.find(boneId);
			if(it == boneCaches.end())
				it = boneCaches.insert(std::make_pair(boneId, pragma::util::make_shared<BoneHitboxBvhCache>())).first;
			auto meshBvhCache = pragma::util::make_shared<MeshHitboxBvhCache>();
			meshBvhCache->bvhTree = std::move(boneMeshInfo->meshBvhTree);
			meshBvhCache->bvhTriToOriginalTri = std::move(boneMeshInfo->usedTris);
			meshBvhCache->mesh = boneMeshInfo->subMesh;

			auto &boneCache = *it->second;
			boneCache.meshCache[boneMeshInfo->meshUuid] = meshBvhCache;
		}
	}
}
std::shared_future<void> pragma::bvh::HitboxBvhCache::GenerateModelCache(const ModelName &mdlName, asset::Model &mdl)
{
	auto normName = pragma::asset::get_normalized_path(mdlName, asset::Type::Model);
	auto it = m_modelBvhCache.find(normName);
	if(it != m_modelBvhCache.end())
		return it->second->task;

	PrepareModel(mdl);

	auto &builder = m_builder;
	auto mdlCache = pragma::util::make_shared<ModelHitboxBvhCache>();
	auto buildModelTask = builder.GetThreadPool().submit_task([this, &builder, &mdl, &mdlCache = *mdlCache]() {
		auto task = builder.BuildModel(mdl);
		InitializeModelHitboxBvhCache(mdl, task, mdlCache);
		mdlCache.complete = true;
	});
	mdlCache->task = buildModelTask.share();
	m_modelBvhCache.insert(std::make_pair(normName, mdlCache));
	return mdlCache->task;
}

CHitboxBvhComponent::CHitboxBvhComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent)
{
	if(g_hbBvhCount++ == 0) {
		g_hbThreadPool = std::make_unique<BS::light_thread_pool>(10);
		g_hbBvhCache = pragma::util::make_shared<bvh::HitboxBvhCache>(GetGame());
	}
}
CHitboxBvhComponent::~CHitboxBvhComponent()
{
	Reset();
	if(--g_hbBvhCount == 0) {
		g_hbThreadPool = {};
		g_hbBvhCache = nullptr;
	}
}
void CHitboxBvhComponent::WaitForHitboxBvhUpdate()
{
	if(m_hitboxBvhUpdate.valid())
		m_hitboxBvhUpdate.wait();
}
pragma::bvh::HitboxBvhCache &CHitboxBvhComponent::GetGlobalBvhCache() const { return *g_hbBvhCache; }
void CHitboxBvhComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CHitboxBvhComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto animC = GetEntity().AddComponent<CAnimatedComponent>();
	if(animC.valid())
		animC->SetSkeletonUpdateCallbacksEnabled(true);
	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { OnModelChanged(); });
	BindEventUnhandled(cModelComponent::EVENT_ON_RENDER_MESHES_UPDATED, [this](std::reference_wrapper<ComponentEvent> evData) { InitializeHitboxMeshBvhs(); });
	BindEventUnhandled(cAnimatedComponent::EVENT_ON_SKELETON_UPDATED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateHitboxBvh(); });

	auto intersectionHandlerC = GetEntity().AddComponent<IntersectionHandlerComponent>();
	if(intersectionHandlerC.valid()) {
		IntersectionHandlerComponent::IntersectionHandler intersectionHandler {};
		intersectionHandler.userData = this;
		intersectionHandler.intersectionTest = [](void *userData, const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo) -> bool { return static_cast<CHitboxBvhComponent *>(userData)->IntersectionTest(origin, dir, minDist, maxDist, outHitInfo); };
		intersectionHandler.intersectionTestAabb = [](void *userData, const Vector3 &min, const Vector3 &max, IntersectionInfo *outIntersectionInfo) -> bool { return static_cast<CHitboxBvhComponent *>(userData)->IntersectionTestAabb(min, max, outIntersectionInfo); };
		intersectionHandler.intersectionTestKDop = [](void *userData, const std::vector<math::Plane> &planes, IntersectionInfo *outIntersectionInfo) -> bool { return static_cast<CHitboxBvhComponent *>(userData)->IntersectionTestKDop(planes, outIntersectionInfo); };
		intersectionHandlerC->SetIntersectionHandler(intersectionHandler);
	}
}

void CHitboxBvhComponent::InitializeBvh()
{
	if(!GetEntity().IsSpawned())
		return;
	InitializeModel();
	InitializeHitboxBvh();
	DebugDraw();
}

void CHitboxBvhComponent::Reset()
{
	WaitForHitboxBvhUpdate();
	m_hitboxBvhUpdate = {};
	m_hitboxMeshCacheTask = {};
	m_hitboxMeshBvhCaches.clear();
	m_hitboxBvh = nullptr;
	m_hitboxBvhUpdatePoses.clear();
}

void CHitboxBvhComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	InitializeBvh();
}

void CHitboxBvhComponent::OnModelChanged()
{
	Reset();
	InitializeBvh();
}

bool CHitboxBvhComponent::InitializeModel()
{
	auto &ent = GetEntity();
	auto &mdl = ent.GetModel();
	if(!mdl)
		return false;

	auto mdlName = ent.GetModelName();
	auto &globalBvhCache = GetGlobalBvhCache();
	auto *mdlCache = globalBvhCache.GetModelCache(mdlName);
	if(mdlCache) {
		// Cache already exists
		InitializeHitboxMeshBvhs();
		return true;
	}

	// Cache doesn't exist, we'll have to generate it and delay the initialization of the BVHs
	m_hitboxMeshCacheTask = globalBvhCache.GenerateModelCache(mdlName, *mdl);
	return true;
}

void CHitboxBvhComponent::UpdateHitboxBvh()
{
	if(!m_hitboxBvh)
		return;
	auto *animC = static_cast<CAnimatedComponent *>(GetEntity().GetAnimatedComponent().get());
	if(!animC)
		return;

	// Note: This is called on the animation thread (which is executed during rendering), but we don't need a mutex
	// as raycasts must not occur during rendering anyway.

	auto &bonePoses = animC->GetProcessedBones();
	auto &hitboxBvh = m_hitboxBvh;
	auto &bvh = m_hitboxBvh->bvh;

	WaitForHitboxBvhUpdate();
	m_hitboxBvhUpdatePoses.resize(bonePoses.size());
	memcpy(m_hitboxBvhUpdatePoses.data(), bonePoses.data(), util::size_of_container(bonePoses));
	auto &updatePoses = m_hitboxBvhUpdatePoses;
	m_hitboxBvhUpdate = g_hbThreadPool->submit_task([&updatePoses, &hitboxBvh, &bvh]() {
		bvh.refit([&bvh, &hitboxBvh, &updatePoses](bvh::Node &node) {
			auto begin = node.index.first_id();
			auto end = begin + node.index.prim_count();
			for(size_t i = begin; i < end; ++i) {
				size_t j = bvh.prim_ids[i];

				auto &hbObb = hitboxBvh->primitives[j];
				if(hbObb.boneId >= updatePoses.size())
					continue;
				auto &pose = updatePoses[hbObb.boneId];
				Vector3 origin;
				auto bbox = hbObb.ToBvhBBox(pose, origin);
				node.set_bbox(bbox);
			}
		});
	});
}

void CHitboxBvhComponent::InitializeHitboxBvh()
{
	WaitForHitboxBvhUpdate();

	auto &ent = GetEntity();
	auto animC = ent.GetAnimatedComponent();
	if(animC.expired())
		return;
	auto &mdl = ent.GetModel();
	auto &hitboxes = mdl->GetHitboxes();
	auto bvhTree = std::make_unique<bvh::ObbBvhTree>();
	auto &hitboxObbs = bvhTree->primitives;
	hitboxObbs.reserve(hitboxes.size());
	auto numBones = mdl->GetSkeleton().GetBoneCount();
	for(auto &pair : hitboxes) {
		if(pair.first >= numBones)
			continue;
		auto it = m_hitboxMeshBvhCaches.find(pair.first);
		if(it == m_hitboxMeshBvhCaches.end())
			continue; // No mesh for this hitbox
		auto &hb = pair.second;
		hitboxObbs.push_back({hb.min, hb.max});
		auto &hbObb = hitboxObbs.back();
		hbObb.boneId = pair.first;
	}
	if(hitboxObbs.empty()) {
		m_hitboxBvh = nullptr;
		return;
	}
	bvhTree->InitializeBvh(animC->GetProcessedBones());
	m_hitboxBvh = std::move(bvhTree);
}

void CHitboxBvhComponent::InitializeHitboxMeshBvhs()
{
	m_hitboxMeshBvhCaches.clear();
	auto &ent = GetEntity();
	auto *mdlC = ent.GetModelComponent();
	if(!mdlC)
		return;
	auto &mdl = ent.GetModel();
	auto mdlName = ent.GetModelName();
	auto &bvhCache = GetGlobalBvhCache();
	auto *cache = bvhCache.GetModelCache(mdlName);
	if(!cache)
		return;
	auto numLods = mdl->GetLODCount();
	if(numLods == 0)
		return;
	auto &lastLod = mdl->GetLODs().back();
	std::vector<std::shared_ptr<geometry::ModelSubMesh>> lodMeshes;
	mdl->GetBodyGroupMeshes(mdlC->GetBodyGroups(), lastLod.lod, lodMeshes);

	std::unordered_set<std::string> renderMeshUuids;
	renderMeshUuids.reserve(lodMeshes.size());
	for(auto &subMesh : lodMeshes)
		renderMeshUuids.insert(util::uuid_to_string(subMesh->GetUuid()));
	// Get the BVH caches for the current body groups
	for(auto &[boneId, boneCache] : cache->boneCache) {
		std::vector<std::shared_ptr<bvh::MeshHitboxBvhCache>> *boneMeshCache = nullptr;
		for(auto &[uuid, meshCache] : boneCache->meshCache) {
			if(renderMeshUuids.find(uuid) == renderMeshUuids.end())
				continue;
			if(boneMeshCache == nullptr) {
				auto &rtCache = m_hitboxMeshBvhCaches[boneId] = std::vector<std::shared_ptr<bvh::MeshHitboxBvhCache>> {};
				rtCache.reserve(boneCache->meshCache.size());
				boneMeshCache = &rtCache;
			}
			boneMeshCache->push_back(meshCache);
		}
	}
}

static void draw_mesh(const pragma::bvh::MeshBvhTree &bvhTree, const math::ScaledTransform &pose, const Color &color, const Color &outlineColor, float duration = 0.1f)
{
	std::vector<Vector3> verts;
	verts.reserve(bvhTree.primitives.size() * 3);
	for(auto &tri : bvhTree.primitives) {
		verts.push_back(pragma::bvh::from_bvh_vector(tri.p0));
		verts.push_back(pragma::bvh::from_bvh_vector(tri.p1));
		verts.push_back(pragma::bvh::from_bvh_vector(tri.p2));
	}
	auto o = debug::DebugRenderer::DrawMesh(verts, {color, outlineColor, duration});
	if(o)
		o->SetPose(pose);
}

bool CHitboxBvhComponent::IntersectionTestAabb(const Vector3 &min, const Vector3 &max, IntersectionInfo *outIntersectionInfo) const
{
	auto &ent = static_cast<const ecs::CBaseEntity &>(GetEntity());
	auto *renderC = ent.GetRenderComponent();
	if(!renderC)
		return false;
	auto animC = ent.GetAnimatedComponent();
	if(animC.expired())
		return false;
	auto &renderBounds = renderC->GetUpdatedAbsoluteRenderBounds();
	if(math::intersection::aabb_aabb(min, max, renderBounds.min, renderBounds.max) == math::intersection::Intersect::Outside)
		return false;

	if(!UpdateHitboxMeshCache())
		return false;

	// Move AABB to entity space
	auto entPoseInv = ent.GetPose().GetInverse();
	auto &obbOrigin = entPoseInv.GetOrigin();
	auto &obbRot = entPoseInv.GetRotation();

	// Raycast against our hitbox BVH
	const_cast<CHitboxBvhComponent *>(this)->WaitForHitboxBvhUpdate(); // Ensure hitbox bvh update is complete

	auto &effectiveBonePoses = animC->GetProcessedBones();
	PrimitiveIntersectionInfo hbIntersectionInfo;
	if(!bvh::test_bvh_intersection_with_obb(*m_hitboxBvh, effectiveBonePoses, obbOrigin, obbRot, min, max, 0u, &hbIntersectionInfo))
		return false;

	auto isPrimitiveIntersectionInfo = outIntersectionInfo != nullptr && typeid(*outIntersectionInfo) == typeid(PrimitiveIntersectionInfo);
	auto isMeshIntersectionInfo = outIntersectionInfo != nullptr && typeid(*outIntersectionInfo) == typeid(MeshIntersectionInfo);

	std::unique_ptr<bvh::IntersectionCache> intersectionCache {};
	if(isPrimitiveIntersectionInfo || isMeshIntersectionInfo)
		intersectionCache = std::make_unique<bvh::IntersectionCache>();

	auto hasHit = false;
	for(auto hbIdx : hbIntersectionInfo.primitives) {
		auto &hObb = m_hitboxBvh->primitives[hbIdx];

		auto it = m_hitboxMeshBvhCaches.find(hObb.boneId);
		if(it == m_hitboxMeshBvhCaches.end())
			continue;
		auto boneId = hObb.boneId;
		if(boneId >= effectiveBonePoses.size())
			continue;
		// Move OBB to bone space
		auto obbPoseBs = effectiveBonePoses[boneId].GetInverse() * entPoseInv;

		auto &hitboxBvhInfos = it->second;
		std::optional<float> closestDistance {};
		for(auto &hitboxBvhInfo : hitboxBvhInfos) {
			auto &meshBvh = hitboxBvhInfo->bvhTree;

			MeshIntersectionInfo meshIntersectionInfo;
			if(!bvh::test_bvh_intersection_with_obb(*meshBvh, obbPoseBs.GetOrigin(), obbPoseBs.GetRotation(), min, max, 0u, &meshIntersectionInfo))
				continue;

			if(isMeshIntersectionInfo) {
				if(!bvh::test_bvh_intersection_with_obb(*meshBvh, obbPoseBs.GetOrigin(), obbPoseBs.GetRotation(), min, max))
					continue;
				auto *outMeshIntersectionInfo = static_cast<MeshIntersectionInfo *>(outIntersectionInfo);
				if(outMeshIntersectionInfo->meshInfos.size() == outMeshIntersectionInfo->meshInfos.capacity())
					outMeshIntersectionInfo->meshInfos.reserve(outMeshIntersectionInfo->meshInfos.size() * 2 + 10);
				outMeshIntersectionInfo->meshInfos.push_back({hitboxBvhInfo->mesh.get(), const_cast<ecs::BaseEntity *>(&GetEntity())});
				hasHit = true;
				continue;
			}

			if(isPrimitiveIntersectionInfo) {
				PrimitiveIntersectionInfo primIntersectionInfo;
				if(!bvh::test_bvh_intersection_with_obb(*meshBvh, obbPoseBs.GetOrigin(), obbPoseBs.GetRotation(), min, max, 0u, &primIntersectionInfo))
					continue;
				auto *outPrimitiveIntersectionInfo = static_cast<PrimitiveIntersectionInfo *>(outIntersectionInfo);
				auto offset = outPrimitiveIntersectionInfo->primitives.size();
				outPrimitiveIntersectionInfo->primitives.resize(offset + primIntersectionInfo.primitives.size());
				memcpy(outPrimitiveIntersectionInfo->primitives.data() + offset, primIntersectionInfo.primitives.data(), util::size_of_container(primIntersectionInfo.primitives));
				intersectionCache->meshRanges.push_back({*hitboxBvhInfo->mesh, offset, outPrimitiveIntersectionInfo->primitives.size()});
				hasHit = true;
				continue;
			}
		}
	}
	return hasHit;
}
bool CHitboxBvhComponent::IntersectionTestKDop(const std::vector<math::Plane> &planes, IntersectionInfo *outIntersectionInfo) const
{
	auto &ent = static_cast<const ecs::CBaseEntity &>(GetEntity());
	auto *renderC = ent.GetRenderComponent();
	if(!renderC)
		return false;
	auto animC = ent.GetAnimatedComponent();
	if(animC.expired())
		return false;
	auto &renderBounds = renderC->GetLocalRenderBounds();
	if(math::intersection::aabb_in_plane_mesh(renderBounds.min, renderBounds.max, planes.begin(), planes.end()) == math::intersection::Intersect::Outside)
		return false;

	if(!UpdateHitboxMeshCache())
		return false;

	// Raycast against our hitbox BVH
	const_cast<CHitboxBvhComponent *>(this)->WaitForHitboxBvhUpdate(); // Ensure hitbox bvh update is complete

	auto &effectiveBonePoses = animC->GetProcessedBones();
	PrimitiveIntersectionInfo hbIntersectionInfo;
	if(!bvh::test_bvh_intersection_with_kdop(*m_hitboxBvh, effectiveBonePoses, planes, 0u, &hbIntersectionInfo))
		return false;

	// Move kdop to entity space
	auto entPoseInv = ent.GetPose().GetInverse();
	auto &kdopOrigin = entPoseInv.GetOrigin();
	auto &kdopRot = entPoseInv.GetRotation();

	auto isPrimitiveIntersectionInfo = outIntersectionInfo != nullptr && typeid(*outIntersectionInfo) == typeid(PrimitiveIntersectionInfo);
	auto isMeshIntersectionInfo = outIntersectionInfo != nullptr && typeid(*outIntersectionInfo) == typeid(MeshIntersectionInfo);

	std::unique_ptr<bvh::IntersectionCache> intersectionCache {};
	if(isPrimitiveIntersectionInfo || isMeshIntersectionInfo)
		intersectionCache = std::make_unique<bvh::IntersectionCache>();

	auto hasHit = false;
	for(auto hbIdx : hbIntersectionInfo.primitives) {
		auto &hObb = m_hitboxBvh->primitives[hbIdx];

		auto it = m_hitboxMeshBvhCaches.find(hObb.boneId);
		if(it == m_hitboxMeshBvhCaches.end())
			continue;
		auto boneId = hObb.boneId;
		if(boneId >= effectiveBonePoses.size())
			continue;
		auto invBonePose = effectiveBonePoses[boneId].GetInverse();
		// Move planes to bone space
		auto planesBs = planes;
		for(auto &planeBs : planesBs)
			planeBs = invBonePose * planeBs;

		auto &hitboxBvhInfos = it->second;
		std::optional<float> closestDistance {};
		for(auto &hitboxBvhInfo : hitboxBvhInfos) {
			auto &meshBvh = hitboxBvhInfo->bvhTree;

			if(!outIntersectionInfo) {
				if(bvh::test_bvh_intersection_with_kdop(*meshBvh, planesBs, 0u))
					return true;
				continue;
			}

			if(isMeshIntersectionInfo) {
				if(!bvh::test_bvh_intersection_with_kdop(*meshBvh, planesBs))
					continue;
				auto *outMeshIntersectionInfo = static_cast<MeshIntersectionInfo *>(outIntersectionInfo);
				if(outMeshIntersectionInfo->meshInfos.size() == outMeshIntersectionInfo->meshInfos.capacity())
					outMeshIntersectionInfo->meshInfos.reserve(outMeshIntersectionInfo->meshInfos.size() * 2 + 10);
				outMeshIntersectionInfo->meshInfos.push_back({hitboxBvhInfo->mesh.get(), const_cast<ecs::BaseEntity *>(&GetEntity())});
				hasHit = true;
				continue;
			}

			if(isPrimitiveIntersectionInfo) {
				PrimitiveIntersectionInfo primIntersectionInfo;
				if(!bvh::test_bvh_intersection_with_kdop(*meshBvh, planesBs, 0u, &primIntersectionInfo))
					continue;
				auto *outPrimitiveIntersectionInfo = static_cast<PrimitiveIntersectionInfo *>(outIntersectionInfo);
				auto offset = outPrimitiveIntersectionInfo->primitives.size();
				outPrimitiveIntersectionInfo->primitives.resize(offset + primIntersectionInfo.primitives.size());
				memcpy(outPrimitiveIntersectionInfo->primitives.data() + offset, primIntersectionInfo.primitives.data(), util::size_of_container(primIntersectionInfo.primitives));
				intersectionCache->meshRanges.push_back({*hitboxBvhInfo->mesh, offset, outPrimitiveIntersectionInfo->primitives.size()});
				hasHit = true;
				continue;
			}
		}
	}
	return hasHit;
}

bool CHitboxBvhComponent::UpdateHitboxMeshCache() const
{
	if(m_hitboxBvh)
		return true;
	// Lazy initialization
	if(m_hitboxMeshCacheTask.valid()) {
		m_hitboxMeshCacheTask.wait();
		auto *c = const_cast<CHitboxBvhComponent *>(this);
		c->m_hitboxMeshCacheTask = {};
		c->InitializeHitboxMeshBvhs();
		c->InitializeHitboxBvh();
	}
	return m_hitboxBvh != nullptr;
}

bool CHitboxBvhComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo, const bvh::DebugDrawInfo *debugDrawInfo) const
{
	auto &ent = static_cast<const ecs::CBaseEntity &>(GetEntity());
	auto *renderC = ent.GetRenderComponent();
	if(!renderC)
		return false;

	auto animC = ent.GetAnimatedComponent();
	if(animC.expired())
		return false;

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("hitbox_bvh_raycast");
#endif
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	pragma::util::ScopeGuard sgVtune {[]() { ::debug::get_domain().EndTask(); }};
#endif

	auto &renderBounds = renderC->GetLocalRenderBounds();
	float t;
	if(math::intersection::line_aabb(origin, dir, renderBounds.min, renderBounds.max, &t) != math::intersection::Result::Intersect || t > maxDist)
		return false; // Ray doesn't intersect with render bounds, so we can quit early

	if(!UpdateHitboxMeshCache())
		return false;

	auto &effectiveBonePoses = animC->GetProcessedBones();

	auto debugDraw = (debugDrawInfo && debugDrawInfo->flags != bvh::DebugDrawInfo::Flags::None);
	if(debugDraw)
		const_cast<bvh::DebugDrawInfo *>(debugDrawInfo)->basePose = GetEntity().GetPose();

	// Raycast against our hitbox BVH
	std::vector<bvh::ObbBvhTree::HitData> hits;
	const_cast<CHitboxBvhComponent *>(this)->WaitForHitboxBvhUpdate(); // Ensure hitbox bvh update is complete

	auto res = m_hitboxBvh->Raycast(origin, dir, minDist, maxDist, effectiveBonePoses, hits, debugDrawInfo);
	if(!res)
		return false;

	// Now we can do a more precise raycast against the hitbox mesh bvh trees
	for(auto &hitData : hits) {
		auto &hObb = m_hitboxBvh->primitives[hitData.primitiveIndex];
		auto it = m_hitboxMeshBvhCaches.find(hObb.boneId);
		if(it == m_hitboxMeshBvhCaches.end())
			continue;
		auto boneId = hObb.boneId;
		if(boneId >= effectiveBonePoses.size())
			continue;
		auto bonePoseInv = effectiveBonePoses[boneId].GetInverse();

		// Move ray to bone space
		auto originBs = bonePoseInv * origin;
		auto dirBs = dir;
		uvec::rotate(&dirBs, bonePoseInv.GetRotation());

		auto &hitboxBvhInfos = it->second;
		std::optional<float> closestDistance {};
		for(auto &hitboxBvhInfo : hitboxBvhInfos) {
			auto &meshBvh = hitboxBvhInfo->bvhTree;

			bvh::MeshBvhTree::HitData meshHitData;
			auto res = meshBvh->Raycast(originBs, dirBs, minDist, maxDist, meshHitData);
			if(debugDraw) {
				if(math::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawTraversedMeshesBit) || (res && math::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawHitMeshesBit))) {
					auto color = res ? Color {0, 255, 0, 64} : Color {255, 0, 0, 64};
					draw_mesh(*meshBvh, debugDrawInfo->basePose * effectiveBonePoses[boneId], color, colors::White, debugDrawInfo->duration);
				}
			}
			if(!res)
				continue;
			if(meshHitData.primitiveIndex >= hitboxBvhInfo->bvhTriToOriginalTri.size())
				continue; // Unreachable
			if(closestDistance && meshHitData.t >= *closestDistance)
				continue;
			closestDistance = meshHitData.t;

			outHitInfo.mesh = hitboxBvhInfo->mesh;
			outHitInfo.distance = meshHitData.t * maxDist;
			outHitInfo.primitiveIndex = hitboxBvhInfo->bvhTriToOriginalTri[meshHitData.primitiveIndex];
			outHitInfo.u = meshHitData.u;
			outHitInfo.v = meshHitData.v;
			outHitInfo.t = meshHitData.t;
		}
		outHitInfo.entity = GetEntity().GetHandle();
		if(closestDistance.has_value())
			return true;
	}
	return false;
}

void CHitboxBvhComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	auto intersectionHandlerC = GetEntity().GetComponent<IntersectionHandlerComponent>();
	if(intersectionHandlerC.valid()) {
		intersectionHandlerC->ClearIntersectionHandler();
	}
}

void CHitboxBvhComponent::DebugDrawHitboxMeshes(animation::BoneId boneId, float duration) const
{
	auto &ent = GetEntity();
	auto animC = ent.GetAnimatedComponent();
	if(animC.expired())
		return;
	auto &mdl = ent.GetModel();
	if(!mdl)
		return;
	auto &effectivePoses = animC->GetProcessedBones();
	if(boneId >= effectivePoses.size())
		return;
	auto it = m_hitboxMeshBvhCaches.find(boneId);
	if(it == m_hitboxMeshBvhCaches.end())
		return;
	const std::array<Color, 6> colors {colors::Red, colors::Lime, colors::Blue, colors::Yellow, colors::Cyan, colors::Magenta};
	auto &hitboxBvhInfos = it->second;
	uint32_t colorIdx = 0;
	auto pose = GetEntity().GetPose();
	auto &ref = mdl->GetReference();
	math::ScaledTransform refBonePose;
	ref.GetBonePose(boneId, refBonePose);
	auto relPose = effectivePoses[boneId] * refBonePose.GetInverse();
	pose *= relPose;
	for(auto &hitboxBvhInfo : hitboxBvhInfos) {
		auto &col = colors[colorIdx];
		auto &verts = hitboxBvhInfo->mesh->GetVertices();

		std::vector<Vector3> dbgVerts;
		dbgVerts.reserve(dbgVerts.size() + hitboxBvhInfo->mesh->GetIndexCount());
		hitboxBvhInfo->mesh->VisitIndices([&verts, &pose, &dbgVerts, &hitboxBvhInfo](auto *indexDataSrc, uint32_t numIndicesSrc) {
			dbgVerts.reserve(hitboxBvhInfo->bvhTriToOriginalTri.size());
			for(auto triIdx : hitboxBvhInfo->bvhTriToOriginalTri) {
				auto idx0 = triIdx * 3;
				auto idx1 = idx0 + 1;
				auto idx2 = idx0 + 2;
				if(idx2 >= numIndicesSrc)
					continue;
				idx0 = indexDataSrc[idx0];
				idx1 = indexDataSrc[idx1];
				idx2 = indexDataSrc[idx2];
				dbgVerts.push_back(verts[idx0].position);
				dbgVerts.push_back(verts[idx1].position);
				dbgVerts.push_back(verts[idx2].position);
			}
		});

		auto o = debug::DebugRenderer::DrawMesh(dbgVerts, {col, colors::White, duration});
		if(o)
			o->SetPose(pose);

		colorIdx = (colorIdx + 1) % colors.size();
	}
}

void CHitboxBvhComponent::DebugDraw()
{
	if(!m_hitboxBvh)
		return;
	auto &ent = GetEntity();
	auto animC = ent.GetAnimatedComponent();
	if(animC.expired())
		return;
	auto &effectivePoses = animC->GetProcessedBones();
	auto numBones = effectivePoses.size();
	auto color = colors::Red;
	auto outlineColor = colors::Lime;
	auto duration = 20.f;
	for(auto &hObb : m_hitboxBvh->primitives) {
		if(hObb.boneId >= numBones)
			continue;
		auto pose = hObb.GetPose(effectivePoses);
		auto &pos = pose.GetOrigin();
		debug::DebugRenderInfo renderInfo {color, outlineColor, duration};
		renderInfo.pose.SetOrigin(pos);
		renderInfo.pose.SetRotation(pose.GetRotation());
		debug::DebugRenderer::DrawBox(-hObb.halfExtents, hObb.halfExtents, renderInfo);
	}

	auto &mdl = ent.GetModel();
	auto &ref = mdl->GetReference();
	for(auto &pair : m_hitboxMeshBvhCaches) {
		math::ScaledTransform pose;
		if(!ref.GetBonePose(pair.first, pose))
			continue;
		std::vector<Vector3> dbgMeshVerts;
		auto &hitboxBvhInfos = pair.second;
		for(auto &hitboxBvhInfo : hitboxBvhInfos) {
			dbgMeshVerts.reserve(dbgMeshVerts.size() + hitboxBvhInfo->bvhTree->primitives.size() * 3);
			for(auto &prim : hitboxBvhInfo->bvhTree->primitives) {
				auto &p0 = prim.p0;
				auto &p1 = prim.p1;
				auto &p2 = prim.p2;
				auto v0 = Vector3 {p0[0], p0[1], p0[2]};
				auto v1 = Vector3 {p1[0], p1[1], p1[2]};
				auto v2 = Vector3 {p2[0], p2[1], p2[2]};
				v0 = pose * v0;
				v1 = pose * v1;
				v2 = pose * v2;
				dbgMeshVerts.push_back(v0);
				dbgMeshVerts.push_back(v1);
				dbgMeshVerts.push_back(v2);
			}
		}
		debug::DebugRenderer::DrawMesh(dbgMeshVerts, {color, outlineColor, duration});
	}
}

pragma::bvh::HitboxObb::HitboxObb(const Vector3 &min, const Vector3 &max) : min {min}, max {max} { math::geometry::calc_aabb_extents(min, max, position, halfExtents); }

math::ScaledTransform pragma::bvh::HitboxObb::GetPose(const std::vector<math::ScaledTransform> &effectivePoses) const
{
	auto pose = effectivePoses[boneId];
	pose.TranslateLocal(position);
	return pose;
}

pragma::bvh::BBox pragma::bvh::HitboxObb::ToBvhBBox(const math::ScaledTransform &pose, Vector3 &outOrigin) const
{
	auto [aabbMin, aabbMax] = math::geometry::calc_aabb_around_obb(pose, position, halfExtents);
	return BBox {to_bvh_vector(aabbMin), to_bvh_vector(aabbMax)};
}

bool pragma::bvh::ObbBvhTree::DoInitializeBvh(Executor &executor, ::bvh::v2::DefaultBuilder<Node>::Config &config)
{
	auto numObbs = primitives.size();
	if(numObbs == 0)
		return false;
	auto numBones = m_poses->size();
	std::vector<BBox> bboxes {numObbs};
	std::vector<Vec> centers {numObbs};
	executor.for_each(0, numObbs, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			Vector3 center;
			auto &hObb = primitives[i];
			if(hObb.boneId >= numBones)
				continue;
			bboxes[i] = hObb.ToBvhBBox((*m_poses)[hObb.boneId], center);
			centers[i] = to_bvh_vector(center);
		}
	});

	bvh = ::bvh::v2::DefaultBuilder<Node>::build(GetThreadPool(), bboxes, centers, config);
	return true;
}

void pragma::bvh::ObbBvhTree::InitializeBvh(const std::vector<math::ScaledTransform> &poses)
{
	m_poses = &poses;
	BvhTree::InitializeBvh();
	m_poses = nullptr;
}

bool pragma::bvh::ObbBvhTree::Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, const std::vector<math::ScaledTransform> &bonePoses, std::vector<HitData> &outHits, const DebugDrawInfo *debugDrawInfo)
{
	constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
	constexpr size_t stack_size = 64;
	constexpr bool use_robust_traversal = false;

	std::function<void(const Node &, const Node &)> innerFn = [](const Node &, const Node &) {};
	if(debugDrawInfo && math::is_flag_set(debugDrawInfo->flags, DebugDrawInfo::Flags::DrawTraversedNodesBit)) {
		auto &game = *get_cengine()->GetClientState()->GetGameState();
		innerFn = [&game, debugDrawInfo](const Node &a, const Node &b) {
			auto col = Color {255, 0, 255, 64};
			debug::draw_node(game, a, debugDrawInfo->basePose, col, debugDrawInfo->duration);
			debug::draw_node(game, b, debugDrawInfo->basePose, col, debugDrawInfo->duration);
		};
	}

	auto distDiff = maxDist - minDist;
	::bvh::v2::SmallStack<Bvh::Index, stack_size> stack;
	auto ray = get_ray(origin, dir, minDist, maxDist);
	auto numBones = bonePoses.size();
	bvh.intersect<false, use_robust_traversal>(
	  ray, bvh.get_root().index, stack,
	  [&](size_t begin, size_t end) {
		  for(size_t i = begin; i < end; ++i) {
			  size_t j = bvh.prim_ids[i];

			  auto &hObb = primitives[j];
			  if(hObb.boneId >= numBones)
				  continue;
			  auto &pose = bonePoses[hObb.boneId];
			  float dist;

			  auto tmpDir = dir * maxDist;
			  auto hit = math::intersection::line_obb(origin, tmpDir, hObb.min, hObb.max, &dist, pose.GetOrigin(), pose.GetRotation());
			  if(debugDrawInfo && (math::is_flag_set(debugDrawInfo->flags, DebugDrawInfo::Flags::DrawTraversedLeavesBit) || (math::is_flag_set(debugDrawInfo->flags, DebugDrawInfo::Flags::DrawHitLeavesBit) && hit))) {
				  auto &game = *get_cengine()->GetClientState()->GetGameState();
				  auto col = hit ? Color {0, 255, 0, 64} : Color {0, 0, 255, 64};
				  Vector3 center;
				  auto bbox = hObb.ToBvhBBox(pose, center);
				  debug::draw_node(game, bbox, debugDrawInfo->basePose, col, debugDrawInfo->duration);
			  }
			  if(hit) {
				  dist *= maxDist;
				  if(outHits.size() == outHits.capacity())
					  outHits.reserve(outHits.size() * 2 + 5);
				  HitData hitData {};
				  hitData.primitiveIndex = j;

				  ray.tmax = dist;
				  if(distDiff > 0.0001f)
					  hitData.t = (ray.tmax - minDist) / distDiff;
				  else
					  hitData.t = 0.f;

				  util::insert_sorted(outHits, hitData, [](const HitData &a, const HitData &b) { return a.t < b.t; });
			  }
		  }
		  return false;
	  },
	  innerFn);
	return !outHits.empty();
}

bool pragma::bvh::test_bvh_intersection(const ObbBvhTree &bvhData, const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const std::function<bool(const HitboxObb &)> &testObb, size_t nodeIdx, IntersectionInfo *outIntersectionInfo)
{
	auto &bvh = bvhData.bvh;
	auto &node = bvh.nodes[nodeIdx];
	constexpr size_t stack_size = 64;
	::bvh::v2::SmallStack<Bvh::Index, stack_size> stack;
	auto hasAnyHit = false;

	auto primitivesRequired = outIntersectionInfo != nullptr && typeid(*outIntersectionInfo) == typeid(PrimitiveIntersectionInfo);
	auto traverse = [&]<bool ReturnOnFirstHit>() {
		bvh.traverse_top_down<ReturnOnFirstHit>(
		  bvh.get_root().index, stack,
		  [outIntersectionInfo, primitivesRequired, &bvhData, &testObb, &hasAnyHit](size_t begin, size_t end) {
			  auto hasHit = false;
			  for(auto i = begin; i < end; ++i) {
				  auto primIdx = i;

				  auto &prim = bvhData.primitives[primIdx];
				  auto res = testObb(prim);
				  if(res) {
					  if(!primitivesRequired)
						  return true;
					  auto *primIntersectionInfo = static_cast<PrimitiveIntersectionInfo *>(outIntersectionInfo);
					  if(primIntersectionInfo->primitives.size() == primIntersectionInfo->primitives.capacity())
						  primIntersectionInfo->primitives.reserve(primIntersectionInfo->primitives.size() * 1.75);
					  primIntersectionInfo->primitives.push_back(primIdx);
					  hasHit = true;
					  hasAnyHit = true;
				  }
			  }
			  return hasHit;
		  },
		  [&testAabb](const Node &left, const Node &right) { return test_node_aabb_intersection(testAabb, left, right); });
	};
	if(!outIntersectionInfo)
		traverse.template operator()<true>();
	else
		traverse.template operator()<false>();
	return hasAnyHit;
}
bool pragma::bvh::test_bvh_intersection_with_obb(const ObbBvhTree &bvhData, const std::vector<math::ScaledTransform> &effectivePoses, const Vector3 &origin, const Quat &rot, const Vector3 &min, const Vector3 &max, size_t nodeIdx, IntersectionInfo *outIntersectionInfo)
{
	auto planes = math::geometry::get_obb_planes(origin, rot, min, max);
	auto pose = math::Transform {origin, rot};
	return test_bvh_intersection(
	  bvhData, [&origin, &rot, &min, &max, &planes](const Vector3 &aabbMin, const Vector3 &aabbMax) -> bool { return math::intersection::aabb_in_plane_mesh(aabbMin, aabbMax, planes.begin(), planes.end()) != math::intersection::Intersect::Outside; },
	  [&min, &max, &effectivePoses, pose](const HitboxObb &hObb) -> bool {
		  auto poseB = hObb.GetPose(effectivePoses);
		  return math::intersection::obb_obb(pose, min, max, poseB, hObb.min, hObb.max) != math::intersection::Intersect::Outside;
	  },
	  nodeIdx, outIntersectionInfo);
}
bool pragma::bvh::test_bvh_intersection_with_aabb(const ObbBvhTree &bvhData, const std::vector<math::ScaledTransform> &effectivePoses, const Vector3 &min, const Vector3 &max, size_t nodeIdx, IntersectionInfo *outIntersectionInfo)
{
	return test_bvh_intersection(
	  bvhData, [&min, &max](const Vector3 &aabbMin, const Vector3 &aabbMax) -> bool { return math::intersection::aabb_aabb(min, max, aabbMin, aabbMax) != math::intersection::Intersect::Outside; },
	  [&min, &max, &effectivePoses](const HitboxObb &prim) -> bool {
		  auto pose = prim.GetPose(effectivePoses);
		  return math::intersection::aabb_obb(min, max, pose.GetOrigin(), pose.GetRotation(), prim.min, prim.max) != math::intersection::Intersect::Outside;
	  },
	  nodeIdx, outIntersectionInfo);
}
bool pragma::bvh::test_bvh_intersection_with_kdop(const ObbBvhTree &bvhData, const std::vector<math::ScaledTransform> &effectivePoses, const std::vector<math::Plane> &kdop, size_t nodeIdx, IntersectionInfo *outIntersectionInfo)
{
	return test_bvh_intersection(
	  bvhData, [&kdop](const Vector3 &aabbMin, const Vector3 &aabbMax) -> bool { return math::intersection::aabb_in_plane_mesh(aabbMin, aabbMax, kdop.begin(), kdop.end()) != math::intersection::Intersect::Outside; },
	  [&kdop, &effectivePoses](const HitboxObb &prim) -> bool {
		  // OBB-KDOP intersection is not yet implemented, so we're calculating an AABB for the OBB and doing a AABB-KDOP intersection
		  // for the time being.
		  // TODO: Change this once OBB-KDOP intersection is implemented!
		  auto pose = prim.GetPose(effectivePoses);
		  auto [aabbMin, aabbMax] = math::geometry::calc_aabb_around_obb(pose, prim.position, prim.halfExtents);
		  return math::intersection::aabb_in_plane_mesh(aabbMin, aabbMax, kdop.begin(), kdop.end()) != math::intersection::Intersect::Outside;
	  },
	  nodeIdx, outIntersectionInfo);
}
