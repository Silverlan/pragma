/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_hitbox_bvh_component.hpp"
#include "pragma/entities/components/hitbox_mesh_bvh_builder.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/logging.hpp"
#include <pragma/entities/components/bvh_data.hpp>
#include <pragma/entities/components/util_bvh.hpp>
#include <pragma/debug/intel_vtune.hpp>
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <panima/skeleton.hpp>
#include <panima/bone.hpp>
#include <panima/skeleton.hpp>
#include <pragma/entities/components/base_bvh_component.hpp>
#include <pragma/asset/util_asset.hpp>
#include <sharedutils/BS_thread_pool.hpp>
#include <mathutil/boundingvolume.h>
#include <bvh/v2/stack.h>
#include <ranges>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

static spdlog::logger &LOGGER = pragma::register_logger("bvh");

#pragma optimize("", off)
static std::shared_ptr<pragma::bvh::HitboxBvhCache> g_hbBvhCache {};
static std::unique_ptr<BS::thread_pool> g_hbThreadPool {};
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
	auto normName = pragma::asset::get_normalized_path(mdlName, pragma::asset::Type::Model);
	auto it = m_modelBvhCache.find(normName);
	return (it != m_modelBvhCache.end() && it->second->complete) ? it->second.get() : nullptr;
}
void pragma::bvh::HitboxBvhCache::PrepareModel(Model &mdl)
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
void pragma::bvh::HitboxBvhCache::InitializeModelHitboxBvhCache(Model &mdl, const HitboxMeshBvhBuildTask &buildTask, ModelHitboxBvhCache &mdlHbBvhCache)
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
				it = boneCaches.insert(std::make_pair(boneId, std::make_shared<pragma::bvh::BoneHitboxBvhCache>())).first;
			auto meshBvhCache = std::make_shared<pragma::bvh::MeshHitboxBvhCache>();
			meshBvhCache->bvhTree = std::move(boneMeshInfo->meshBvhTree);
			meshBvhCache->bvhTriToOriginalTri = std::move(boneMeshInfo->usedTris);
			meshBvhCache->mesh = boneMeshInfo->subMesh;

			auto &boneCache = *it->second;
			boneCache.meshCache[boneMeshInfo->meshUuid] = meshBvhCache;
		}
	}
}
std::shared_future<void> pragma::bvh::HitboxBvhCache::GenerateModelCache(const ModelName &mdlName, Model &mdl)
{
	auto normName = pragma::asset::get_normalized_path(mdlName, pragma::asset::Type::Model);
	auto it = m_modelBvhCache.find(normName);
	if(it != m_modelBvhCache.end())
		return it->second->task;

	PrepareModel(mdl);

	auto &builder = m_builder;
	auto mdlCache = std::make_shared<ModelHitboxBvhCache>();
	auto buildModelTask = builder.GetThreadPool().submit_task([this, &builder, &mdl, &mdlCache = *mdlCache]() {
		auto task = builder.BuildModel(mdl);
		InitializeModelHitboxBvhCache(mdl, task, mdlCache);
		mdlCache.complete = true;
	});
	mdlCache->task = buildModelTask.share();
	m_modelBvhCache.insert(std::make_pair(normName, mdlCache));
	return mdlCache->task;
}

CHitboxBvhComponent::CHitboxBvhComponent(BaseEntity &ent) : BaseEntityComponent(ent)
{
	if(g_hbBvhCount++ == 0) {
		g_hbThreadPool = std::make_unique<BS::thread_pool>(10);
		g_hbBvhCache = std::make_shared<pragma::bvh::HitboxBvhCache>(GetGame());
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
void CHitboxBvhComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CHitboxBvhComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnModelChanged(); });
	BindEventUnhandled(CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { InitializeHitboxMeshBvhs(); });
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

	auto &bonePoses = animC->GetProcessedBones();
	auto &hitboxBvh = m_hitboxBvh;
	auto &bvh = m_hitboxBvh->bvh;

	WaitForHitboxBvhUpdate();
	m_hitboxBvhUpdatePoses.resize(bonePoses.size());
	memcpy(m_hitboxBvhUpdatePoses.data(), bonePoses.data(), util::size_of_container(bonePoses));
	auto &updatePoses = m_hitboxBvhUpdatePoses;
	m_hitboxBvhUpdate = g_hbThreadPool->submit_task([&updatePoses, &hitboxBvh, &bvh]() {
		// It's important to process the nodes in reverse order, as this makes
		// sure that children are processed before their parents.
		for(auto &node : std::ranges::reverse_view {bvh.nodes}) {
			if(node.is_leaf()) {
				// refit node according to contents
				auto begin = node.index.first_id;
				auto end = begin + node.index.prim_count;
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
			}
			else {
				auto &left = bvh.nodes[node.index.first_id];
				auto &right = bvh.nodes[node.index.first_id + 1];
				node.set_bbox(left.get_bbox().extend(right.get_bbox()));
			}
		}
		hitboxBvh->Refit();
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
	auto bvhTree = std::make_unique<ObbBvhTree>();
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
	std::vector<std::shared_ptr<ModelSubMesh>> lodMeshes;
	mdl->GetBodyGroupMeshes(mdlC->GetBodyGroups(), lastLod.lod, lodMeshes);

	std::unordered_set<std::string> renderMeshUuids;
	renderMeshUuids.reserve(lodMeshes.size());
	for(auto &subMesh : lodMeshes)
		renderMeshUuids.insert(util::uuid_to_string(subMesh->GetUuid()));
	// Get the BVH caches for the current body groups
	for(auto &[boneId, boneCache] : cache->boneCache) {
		std::vector<std::shared_ptr<pragma::bvh::MeshHitboxBvhCache>> *boneMeshCache = nullptr;
		for(auto &[uuid, meshCache] : boneCache->meshCache) {
			if(renderMeshUuids.find(uuid) == renderMeshUuids.end())
				continue;
			if(boneMeshCache == nullptr) {
				auto &rtCache = m_hitboxMeshBvhCaches[boneId] = std::vector<std::shared_ptr<pragma::bvh::MeshHitboxBvhCache>> {};
				rtCache.reserve(boneCache->meshCache.size());
				boneMeshCache = &rtCache;
			}
			boneMeshCache->push_back(meshCache);
		}
	}
}

static void draw_mesh(const pragma::bvh::MeshBvhTree &bvhTree, const umath::ScaledTransform &pose, const Color &color, const Color &outlineColor, float duration = 0.1f)
{
	std::vector<Vector3> verts;
	verts.reserve(bvhTree.primitives.size() * 3);
	for(auto &tri : bvhTree.primitives) {
		verts.push_back(pose * pragma::bvh::from_bvh_vector(tri.p0));
		verts.push_back(pose * pragma::bvh::from_bvh_vector(tri.p1));
		verts.push_back(pose * pragma::bvh::from_bvh_vector(tri.p2));
	}
	DebugRenderer::DrawMesh(verts, color, outlineColor, duration);
}

bool CHitboxBvhComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, pragma::bvh::HitInfo &outHitInfo, const bvh::DebugDrawInfo *debugDrawInfo)
{
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
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
	util::ScopeGuard sgVtune {[]() { ::debug::get_domain().EndTask(); }};
#endif

	auto &renderBounds = renderC->GetAbsoluteRenderBounds();
	float t;
	if(umath::intersection::line_aabb(origin, dir, renderBounds.min, renderBounds.max, &t) != umath::intersection::Result::Intersect || t > maxDist)
		return false; // Ray doesn't intersect with render bounds, so we can quit early

	if(!m_hitboxBvh) {
		// Lazy initialization
		if(m_hitboxMeshCacheTask.valid()) {
			m_hitboxMeshCacheTask.wait();
			m_hitboxMeshCacheTask = {};
			InitializeHitboxMeshBvhs();
			InitializeHitboxBvh();
		}
		if(!m_hitboxBvh)
			return false;
	}

	auto &effectiveBonePoses = animC->GetProcessedBones();

	// Move ray to entity space
	auto entPoseInv = ent.GetPose().GetInverse();
	auto originEs = entPoseInv * origin;
	auto dirEs = dir;
	uvec::rotate(&dirEs, entPoseInv.GetRotation());

	auto debugDraw = (debugDrawInfo->flags != bvh::DebugDrawInfo::Flags::None);

	// Raycast against our hitbox BVH
	std::vector<ObbBvhTree::HitData> hits;
	WaitForHitboxBvhUpdate(); // Ensure hitbox bvh update is complete

	auto res = m_hitboxBvh->Raycast(originEs, dirEs, minDist, maxDist, effectiveBonePoses, hits, debugDrawInfo);
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
		auto originBs = bonePoseInv * originEs;
		auto dirBs = dirEs;
		uvec::rotate(&dirBs, bonePoseInv.GetRotation());

		auto &hitboxBvhInfos = it->second;
		std::optional<float> closestDistance {};
		for(auto &hitboxBvhInfo : hitboxBvhInfos) {
			auto &meshBvh = hitboxBvhInfo->bvhTree;

			bvh::MeshBvhTree::HitData meshHitData;
			auto res = meshBvh->Raycast(originBs, dirBs, minDist, maxDist, meshHitData);
			if(debugDraw) {
				if(umath::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawTraversedMeshesBit) || (res && umath::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawHitMeshesBit))) {
					auto color = res ? Color {0, 255, 0, 64} : Color {255, 0, 0, 64};
					draw_mesh(*meshBvh, effectiveBonePoses[boneId] * debugDrawInfo->basePose, color, Color::White, debugDrawInfo->duration);
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

void CHitboxBvhComponent::OnRemove() {}

void CHitboxBvhComponent::DebugDrawHitboxMeshes(BoneId boneId, float duration) const
{
	auto it = m_hitboxMeshBvhCaches.find(boneId);
	if(it == m_hitboxMeshBvhCaches.end())
		return;
	const std::array<Color, 6> colors {Color::Red, Color::Lime, Color::Blue, Color::Yellow, Color::Cyan, Color::Magenta};
	auto &hitboxBvhInfos = it->second;
	uint32_t colorIdx = 0;
	for(auto &hitboxBvhInfo : hitboxBvhInfos) {
		auto &col = colors[colorIdx];
		auto &verts = hitboxBvhInfo->mesh->GetVertices();

		std::vector<Vector3> dbgVerts;
		dbgVerts.reserve(dbgVerts.size() + hitboxBvhInfo->mesh->GetIndexCount());
		hitboxBvhInfo->mesh->VisitIndices([&verts, &dbgVerts, &hitboxBvhInfo](auto *indexDataSrc, uint32_t numIndicesSrc) {
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

		DebugRenderer::DrawMesh(dbgVerts, col, Color::White, duration);

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
	auto color = Color::Red;
	auto outlineColor = Color::Lime;
	auto duration = 20.f;
	for(auto &hObb : m_hitboxBvh->primitives) {
		if(hObb.boneId >= numBones)
			continue;
		auto &pose = effectivePoses[hObb.boneId];
		pose.TranslateLocal(hObb.position);
		auto &pos = pose.GetOrigin();
		::DebugRenderer::DrawBox(pos, -hObb.halfExtents, hObb.halfExtents, EulerAngles {pose.GetRotation()}, color, outlineColor, duration);
	}

	auto &mdl = ent.GetModel();
	auto &ref = mdl->GetReference();
	for(auto &pair : m_hitboxMeshBvhCaches) {
		umath::ScaledTransform pose;
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
		::DebugRenderer::DrawMesh(dbgMeshVerts, color, outlineColor, duration);
	}
}

pragma::CHitboxBvhComponent::HitboxObb::HitboxObb(const Vector3 &min, const Vector3 &max) : min {min}, max {max}
{
	position = (max + min) / 2.f;
	halfExtents = (max - min) / 2.f;
}

pragma::bvh::BBox pragma::CHitboxBvhComponent::HitboxObb::ToBvhBBox(const umath::ScaledTransform &pose, Vector3 &outOrigin) const
{
	// Calculate AABB around OBB
	auto rotationMatrix = glm::mat3_cast(pose.GetRotation());
	auto worldX = rotationMatrix * Vector3(1.0f, 0.0f, 0.0f);
	auto worldY = rotationMatrix * Vector3(0.0f, 1.0f, 0.0f);
	auto worldZ = rotationMatrix * Vector3(0.0f, 0.0f, 1.0f);

	auto xAxisExtents = glm::abs(worldX * halfExtents.x);
	auto yAxisExtents = glm::abs(worldY * halfExtents.y);
	auto zAxisExtents = glm::abs(worldZ * halfExtents.z);

	auto posRot = position;
	uvec::rotate(&posRot, pose.GetRotation());
	auto &origin = outOrigin;
	origin = pose.GetOrigin() + posRot;

	auto aabbMin = origin - xAxisExtents - yAxisExtents - zAxisExtents;
	auto aabbMax = origin + xAxisExtents + yAxisExtents + zAxisExtents;
	return pragma::bvh::BBox {pragma::bvh::to_bvh_vector(aabbMin), pragma::bvh::to_bvh_vector(aabbMax)};
}

bool ObbBvhTree::DoInitializeBvh(pragma::bvh::Executor &executor, ::bvh::v2::DefaultBuilder<pragma::bvh::Node>::Config &config)
{
	auto numObbs = primitives.size();
	if(numObbs == 0)
		return false;
	auto numBones = m_poses->size();
	std::vector<pragma::bvh::BBox> bboxes {numObbs};
	std::vector<pragma::bvh::Vec> centers {numObbs};
	executor.for_each(0, numObbs, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			Vector3 center;
			auto &hObb = primitives[i];
			if(hObb.boneId >= numBones)
				continue;
			bboxes[i] = hObb.ToBvhBBox((*m_poses)[hObb.boneId], center);
			centers[i] = bvh::to_bvh_vector(center);
		}
	});

	bvh = ::bvh::v2::DefaultBuilder<pragma::bvh::Node>::build(GetThreadPool(), bboxes, centers, config);
	return true;
}

void ObbBvhTree::InitializeBvh(const std::vector<umath::ScaledTransform> &poses)
{
	m_poses = &poses;
	pragma::bvh::BvhTree::InitializeBvh();
	m_poses = nullptr;
}

bool ObbBvhTree::Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, const std::vector<umath::ScaledTransform> &bonePoses, std::vector<HitData> &outHits, const bvh::DebugDrawInfo *debugDrawInfo)
{
	constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
	constexpr size_t stack_size = 64;
	constexpr bool use_robust_traversal = false;

	std::function<void(const bvh::Node &, const bvh::Node &)> innerFn = [](const bvh::Node &, const bvh::Node &) {};
	if(debugDrawInfo && umath::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawTraversedNodesBit)) {
		auto &game = *pragma::get_cengine()->GetClientState()->GetGameState();
		innerFn = [&game, debugDrawInfo](const bvh::Node &a, const bvh::Node &b) {
			auto col = Color {255, 0, 255, 64};
			bvh::debug::draw_node(game, a, debugDrawInfo->basePose, col, debugDrawInfo->duration);
			bvh::debug::draw_node(game, b, debugDrawInfo->basePose, col, debugDrawInfo->duration);
		};
	}

	auto distDiff = maxDist - minDist;
	::bvh::v2::SmallStack<pragma::bvh::Bvh::Index, stack_size> stack;
	auto ray = pragma::bvh::get_ray(origin, dir, minDist, maxDist);
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
			  auto hit = umath::intersection::line_obb(origin, tmpDir, hObb.min, hObb.max, &dist, pose.GetOrigin(), pose.GetRotation());
			  if(debugDrawInfo && (umath::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawTraversedLeavesBit) || (umath::is_flag_set(debugDrawInfo->flags, bvh::DebugDrawInfo::Flags::DrawHitLeavesBit) && hit))) {
				  auto &game = *pragma::get_cengine()->GetClientState()->GetGameState();
				  auto col = hit ? Color {0, 255, 0, 64} : Color {0, 0, 255, 64};
				  Vector3 center;
				  auto bbox = hObb.ToBvhBBox(pose, center);
				  bvh::debug::draw_node(game, bbox, debugDrawInfo->basePose, col, debugDrawInfo->duration);
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
