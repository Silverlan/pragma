// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <bvh/v2/stack.h>
#include <cassert>

module pragma.shared;

import :entities.components.base_bvh;

using namespace pragma;

void BaseBvhComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseBvhComponent::EVENT_ON_CLEAR_BVH = registerEvent("EVENT_ON_CLEAR_BVH", ComponentEventInfo::Type::Explicit);
	baseBvhComponent::EVENT_ON_BVH_UPDATE_REQUESTED = registerEvent("EVENT_ON_BVH_UPDATE_REQUESTED", ComponentEventInfo::Type::Explicit);
	baseBvhComponent::EVENT_ON_BVH_REBUILT = registerEvent("EVENT_ON_BVH_REBUILT", ComponentEventInfo::Type::Explicit);
}

void BaseBvhComponent::ClearBvh()
{
	InvokeEventCallbacks(baseBvhComponent::EVENT_ON_CLEAR_BVH);
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
	m_bvhDataMutex.lock();
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	m_bvhData = nullptr;
	m_bvhDataMutex.unlock();
}

void BaseBvhComponent::RebuildBvh()
{
	ClearBvh();
	if(m_staticCache.valid()) {
		m_staticCache->SetCacheDirty();
		return;
	}
	DoRebuildBvh();

	InvokeEventCallbacks(baseBvhComponent::EVENT_ON_BVH_REBUILT);
}

void BaseBvhComponent::SetStaticCache(BaseStaticBvhCacheComponent *staticCache) { m_staticCache = staticCache ? staticCache->GetHandle<BaseStaticBvhCacheComponent>() : ComponentHandle<BaseStaticBvhCacheComponent> {}; }

bool BaseBvhComponent::HasBvhData() const
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
	std::scoped_lock lock {m_bvhDataMutex};
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	return m_bvhData != nullptr;
}

std::shared_ptr<pragma::bvh::MeshBvhTree> BaseBvhComponent::SetBvhData(std::shared_ptr<bvh::MeshBvhTree> &bvhData)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
	std::scoped_lock lock {m_bvhDataMutex};
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	auto tmp = bvhData;
	m_bvhData = bvhData;
	return tmp;
}
void BaseBvhComponent::DebugPrint()
{
	auto &bvh = m_bvhData->bvh;
	bvh::debug::print_bvh_tree(bvh);
}
void BaseBvhComponent::DebugDraw()
{
	auto &bvh = m_bvhData->bvh;
	bvh::debug::draw_bvh_tree(GetGame(), bvh, GetEntity().GetPose());
}
void BaseBvhComponent::DebugDrawBvhTree(const Vector3 &origin, const Vector3 &dir, float maxDist, float duration) const
{
	constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
	constexpr size_t stack_size = 64;
	constexpr bool use_robust_traversal = false;

	::bvh::v2::SmallStack<bvh::Bvh::Index, stack_size> stack;
	auto ray = bvh::get_ray(origin, dir, 0.f, maxDist);
	auto &bvh = m_bvhData->bvh;
	auto &game = GetGame();
	auto pose = GetEntity().GetPose();
	bvh.intersect<false, use_robust_traversal>(
	  ray, bvh.get_root().index, stack, [&](size_t begin, size_t end) { return false; },
	  [&game, &pose, duration](const bvh::Node &a, const bvh::Node &b) {
		  auto col = Color {255, 0, 255, 64};
		  bvh::debug::draw_node(game, a, pose, col, duration);
		  bvh::debug::draw_node(game, b, pose, col, duration);
	  });
}
size_t BaseBvhComponent::GetTriangleCount() const { return m_bvhData->primitives.size(); }
std::optional<Vector3> BaseBvhComponent::GetVertex(size_t idx) const
{
	std::scoped_lock lock {m_bvhDataMutex};
	auto primIdx = idx / 3;
	if(primIdx >= m_bvhData->primitives.size())
		return {};
	auto &prim = m_bvhData->primitives[primIdx];
	auto subIdx = idx % 3;
	switch(subIdx) {
	case 0:
		return *reinterpret_cast<const Vector3 *>(&prim.p0.values);
	case 1:
		return *reinterpret_cast<const Vector3 *>(&prim.p1.values);
	case 2:
		return *reinterpret_cast<const Vector3 *>(&prim.p2.values);
	}
	return {};
}

void BaseBvhComponent::GetVertexData(std::vector<bvh::Primitive> &outData) const
{
	std::scoped_lock lock {m_bvhDataMutex};
	outData.resize(m_bvhData->primitives.size());
	memcpy(outData.data(), m_bvhData->primitives.data(), util::size_of_container(outData));
}

void BaseBvhComponent::DeleteRange(bvh::MeshBvhTree &bvhData, size_t start, size_t end)
{
	if(end == start)
		return;
	// We can't actually delete primitives, so we'll just move all of the vertices to a single point to effectively
	// invalidate it
	auto &p = bvhData.primitives[start / 3];
	p = {{p.p0[0], p.p0[1], p.p0[2]}, {p.p0[0], p.p0[1], p.p0[2]}, {p.p0[0], p.p0[1], p.p0[2]}};
	for(size_t i = (start / 3) + 1; i < (end / 3); ++i)
		bvhData.primitives[i] = p;
	bvhData.SetDirty();
}

bool BaseBvhComponent::SetVertexData(bvh::MeshBvhTree &bvhData, const std::vector<bvh::Primitive> &data)
{
	if(bvhData.primitives.size() != data.size())
		return false;
	memcpy(bvhData.primitives.data(), data.data(), util::size_of_container(data));
	bvhData.SetDirty();
	return true;
}

bool BaseBvhComponent::SetVertexData(const std::vector<bvh::Primitive> &data)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
	std::scoped_lock lock {m_bvhDataMutex};
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	return SetVertexData(*m_bvhData, data);
}

bool BaseBvhComponent::ShouldConsiderMesh(const geometry::ModelSubMesh &mesh) { return mesh.GetGeometryType() == geometry::ModelSubMesh::GeometryType::Triangles; }

std::shared_ptr<pragma::bvh::MeshBvhTree> BaseBvhComponent::RebuildBvh(const std::vector<std::shared_ptr<geometry::ModelSubMesh>> &meshes, const BvhBuildInfo *optBvhBuildInfo, std::vector<size_t> *optOutMeshIndices, ecs::BaseEntity *ent)
{
	auto bvhData = std::make_unique<bvh::MeshBvhTree>();

	size_t numVerts = 0;
	bvhData->meshRanges.reserve(meshes.size());
	if(optOutMeshIndices)
		optOutMeshIndices->reserve(meshes.size());
	size_t primitiveOffset = 0;
	for(uint32_t meshIdx = 0; auto &mesh : meshes) {
		if(optBvhBuildInfo && optBvhBuildInfo->isCancelled && optBvhBuildInfo->isCancelled()) {
			++meshIdx;
			return nullptr;
		}
		if(optBvhBuildInfo && optBvhBuildInfo->shouldConsiderMesh && optBvhBuildInfo->shouldConsiderMesh(*mesh, meshIdx) == false) {
			++meshIdx;
			continue;
		}
		bvhData->meshRanges.push_back({});

		auto &rangeInfo = bvhData->meshRanges.back();
		rangeInfo.entity = ent;
		rangeInfo.mesh = mesh;
		rangeInfo.start = primitiveOffset;
		rangeInfo.end = rangeInfo.start + mesh->GetIndexCount();

		numVerts += mesh->GetIndexCount();
		primitiveOffset += mesh->GetIndexCount();
		if(optOutMeshIndices)
			optOutMeshIndices->push_back(meshIdx);
		++meshIdx;
	}

	auto &primitives = bvhData->primitives;
	primitives.resize(numVerts / 3);
	primitiveOffset = 0;
	for(uint32_t meshIdx = 0; auto &mesh : meshes) {
		if(optBvhBuildInfo && optBvhBuildInfo->isCancelled && optBvhBuildInfo->isCancelled())
			return nullptr;
		if(optBvhBuildInfo && optBvhBuildInfo->shouldConsiderMesh && optBvhBuildInfo->shouldConsiderMesh(*mesh, meshIdx) == false) {
			++meshIdx;
			continue;
		}
		auto *pose = (optBvhBuildInfo && optBvhBuildInfo->poses) ? &(*optBvhBuildInfo->poses)[meshIdx] : nullptr;
		auto &verts = mesh->GetVertices();
		mesh->VisitIndices([&verts, primitiveOffset, &primitives, pose](auto *indexDataSrc, uint32_t numIndicesSrc) {
			for(auto i = decltype(numIndicesSrc) {0}; i < numIndicesSrc; i += 3) {
				auto va = verts[indexDataSrc[i]].position;
				auto vb = verts[indexDataSrc[i + 1]].position;
				auto vc = verts[indexDataSrc[i + 2]].position;

				if(pose) {
					va = *pose * va;
					vb = *pose * vb;
					vc = *pose * vc;
				}

				auto &prim = primitives[(primitiveOffset + i) / 3];
				prim = {{va.x, va.y, va.z}, {vb.x, vb.y, vb.z}, {vc.x, vc.y, vc.z}};
			}
		});
		primitiveOffset += mesh->GetIndexCount();
		++meshIdx;
	}

	bvhData->InitializeBvh();
	return std::move(bvhData);
}

std::vector<pragma::bvh::MeshRange> &BaseBvhComponent::GetMeshRanges() { return m_bvhData->meshRanges; }

const std::shared_ptr<pragma::bvh::MeshBvhTree> &BaseBvhComponent::GetUpdatedBvh() const
{
	if(m_sendBvhUpdateRequestOnInteraction)
		InvokeEventCallbacks(baseBvhComponent::EVENT_ON_BVH_UPDATE_REQUESTED);
	return m_bvhData;
}
void BaseBvhComponent::SendBvhUpdateRequestOnInteraction() { m_sendBvhUpdateRequestOnInteraction = true; }
bool BaseBvhComponent::IntersectionTestAabb(const Vector3 &min, const Vector3 &max, IntersectionInfo &outIntersectionInfo) const
{
	auto bvhData = GetUpdatedBvh();
	if(!bvhData || bvhData->primitives.empty())
		return false;

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
	std::scoped_lock lock {m_bvhDataMutex};
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	return test_bvh_intersection_with_aabb(*bvhData, min, max, 0u, &outIntersectionInfo);
}
bool BaseBvhComponent::IntersectionTestAabb(const Vector3 &min, const Vector3 &max) const
{
	auto bvhData = GetUpdatedBvh();
	if(!bvhData || bvhData->primitives.empty())
		return false;

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
	std::scoped_lock lock {m_bvhDataMutex};
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	return test_bvh_intersection_with_aabb(*bvhData, min, max);
}
bool BaseBvhComponent::IntersectionTestKDop(const std::vector<math::Plane> &planes, IntersectionInfo &outIntersectionInfo) const
{
	auto bvhData = GetUpdatedBvh();
	if(!bvhData || bvhData->primitives.empty())
		return false;

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
	std::scoped_lock lock {m_bvhDataMutex};
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	return test_bvh_intersection_with_kdop(*bvhData, planes, 0u, &outIntersectionInfo);
}
bool BaseBvhComponent::IntersectionTestKDop(const std::vector<math::Plane> &planes) const
{
	auto bvhData = GetUpdatedBvh();
	if(!bvhData || bvhData->primitives.empty())
		return false;

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
	std::scoped_lock lock {m_bvhDataMutex};
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	return test_bvh_intersection_with_kdop(*bvhData, planes);
}

bool BaseBvhComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo) const
{
	auto bvhData = GetUpdatedBvh();
	if(!bvhData || bvhData->primitives.empty())
		return false;
	bvh::MeshBvhTree::HitData bvhHitData;
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("bvh_mutex_wait");
#endif
	m_bvhDataMutex.lock();
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	auto hit = bvhData->Raycast(origin, dir, minDist, maxDist, bvhHitData);
	m_bvhDataMutex.unlock();
	if(hit) {
		bvh::MeshRange search {};
		search.start = bvhHitData.primitiveIndex * 3;
		auto it = std::upper_bound(bvhData->meshRanges.begin(), bvhData->meshRanges.end(), search);
		assert(it != bvhData->meshRanges.begin());
		--it;

		auto distance = minDist + (maxDist - minDist) * bvhHitData.t;
		auto &hitInfo = outHitInfo;
		hitInfo.primitiveIndex = bvhHitData.primitiveIndex - it->start / 3;
		hitInfo.distance = distance;
		hitInfo.u = bvhHitData.u;
		hitInfo.v = bvhHitData.v;
		hitInfo.t = bvhHitData.t;
		hitInfo.mesh = it->mesh;
		hitInfo.entity = it->entity ? it->entity->GetHandle() : GetEntity().GetHandle();
		return true;
	}
	return false;
}

const pragma::bvh::MeshRange *BaseBvhComponent::FindPrimitiveMeshInfo(size_t primIdx) const { return m_bvhData->FindMeshRange(primIdx); }

BaseBvhComponent::BaseBvhComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
BaseBvhComponent::~BaseBvhComponent() {}
void BaseBvhComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto intersectionHandlerC = GetEntity().AddComponent<IntersectionHandlerComponent>();
	if(intersectionHandlerC.valid()) {
		IntersectionHandlerComponent::IntersectionHandler intersectionHandler {};
		intersectionHandler.userData = this;
		intersectionHandler.intersectionTest = [](void *userData, const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo) -> bool { return static_cast<BaseBvhComponent *>(userData)->IntersectionTest(origin, dir, minDist, maxDist, outHitInfo); };
		intersectionHandler.intersectionTestAabb = [](void *userData, const Vector3 &min, const Vector3 &max, IntersectionInfo *outIntersectionInfo) -> bool {
			if(outIntersectionInfo)
				return static_cast<BaseBvhComponent *>(userData)->IntersectionTestAabb(min, max, *outIntersectionInfo);
			return static_cast<BaseBvhComponent *>(userData)->IntersectionTestAabb(min, max);
		};
		intersectionHandler.intersectionTestKDop = [](void *userData, const std::vector<math::Plane> &planes, IntersectionInfo *outIntersectionInfo) -> bool {
			if(outIntersectionInfo)
				return static_cast<BaseBvhComponent *>(userData)->IntersectionTestKDop(planes, *outIntersectionInfo);
			return static_cast<BaseBvhComponent *>(userData)->IntersectionTestKDop(planes);
		};
		intersectionHandlerC->SetIntersectionHandler(intersectionHandler);
	}
}

void BaseBvhComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();

	auto intersectionHandlerC = GetEntity().GetComponent<IntersectionHandlerComponent>();
	if(intersectionHandlerC.valid())
		intersectionHandlerC->ClearIntersectionHandler();
}
