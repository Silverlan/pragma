/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_bvh_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_static_bvh_cache_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/debug/intel_vtune.hpp"

using namespace pragma;

ComponentEventId BaseBvhComponent::EVENT_ON_CLEAR_BVH = INVALID_COMPONENT_ID;
ComponentEventId BaseBvhComponent::EVENT_ON_BVH_UPDATE_REQUESTED = INVALID_COMPONENT_ID;
ComponentEventId BaseBvhComponent::EVENT_ON_BVH_REBUILT = INVALID_COMPONENT_ID;
void BaseBvhComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_CLEAR_BVH = registerEvent("EVENT_ON_CLEAR_BVH", ComponentEventInfo::Type::Explicit);
	EVENT_ON_BVH_UPDATE_REQUESTED = registerEvent("EVENT_ON_BVH_UPDATE_REQUESTED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_BVH_REBUILT = registerEvent("EVENT_ON_BVH_REBUILT", ComponentEventInfo::Type::Explicit);
}

void BaseBvhComponent::ClearBvh()
{
	InvokeEventCallbacks(EVENT_ON_CLEAR_BVH);
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

	InvokeEventCallbacks(EVENT_ON_BVH_REBUILT);
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

std::shared_ptr<pragma::bvh::BvhData> BaseBvhComponent::SetBvhData(std::shared_ptr<pragma::bvh::BvhData> &bvhData)
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

void BaseBvhComponent::GetVertexData(std::vector<pragma::bvh::Primitive> &outData) const
{
	std::scoped_lock lock {m_bvhDataMutex};
	outData.resize(m_bvhData->primitives.size());
	memcpy(outData.data(), m_bvhData->primitives.data(), util::size_of_container(outData));
}

static void refit(pragma::bvh::BvhData &bvhData) { bvhData.Refit(); }

void BaseBvhComponent::DeleteRange(pragma::bvh::BvhData &bvhData, size_t start, size_t end)
{
	if(end == start)
		return;
	// We can't actually delete primitives, so we'll just move all of the vertices to a single point to effectively
	// invalidate it
	auto &p = bvhData.primitives[start / 3];
	p = {{p.p0[0], p.p0[1], p.p0[2]}, {p.p0[0], p.p0[1], p.p0[2]}, {p.p0[0], p.p0[1], p.p0[2]}};
	for(size_t i = (start / 3) + 1; i < (end / 3); ++i)
		bvhData.primitives[i] = p;
	refit(bvhData);
}

bool BaseBvhComponent::SetVertexData(pragma::bvh::BvhData &bvhData, const std::vector<pragma::bvh::Primitive> &data)
{
	if(bvhData.primitives.size() != data.size())
		return false;
	memcpy(bvhData.primitives.data(), data.data(), util::size_of_container(data));
	refit(bvhData);
	return true;
}

bool BaseBvhComponent::SetVertexData(const std::vector<pragma::bvh::Primitive> &data)
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

bool BaseBvhComponent::ShouldConsiderMesh(const ModelSubMesh &mesh) { return mesh.GetGeometryType() == ModelSubMesh::GeometryType::Triangles; }

std::shared_ptr<pragma::bvh::BvhData> BaseBvhComponent::RebuildBvh(const std::vector<std::shared_ptr<ModelSubMesh>> &meshes, const BvhBuildInfo *optBvhBuildInfo, std::vector<size_t> *optOutMeshIndices)
{
	auto bvhData = std::make_unique<pragma::bvh::BvhData>();

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

const std::shared_ptr<pragma::bvh::BvhData> &BaseBvhComponent::GetUpdatedBvh() const
{
	if(m_sendBvhUpdateRequestOnInteraction)
		InvokeEventCallbacks(EVENT_ON_BVH_UPDATE_REQUESTED);
	return m_bvhData;
}
void BaseBvhComponent::SendBvhUpdateRequestOnInteraction() { m_sendBvhUpdateRequestOnInteraction = true; }
bool BaseBvhComponent::IntersectionTestAabb(const Vector3 &min, const Vector3 &max, bvh::IntersectionInfo &outIntersectionInfo) const
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
bool BaseBvhComponent::IntersectionTestKDop(const std::vector<umath::Plane> &planes, pragma::bvh::IntersectionInfo &outIntersectionInfo) const
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
bool BaseBvhComponent::IntersectionTestKDop(const std::vector<umath::Plane> &planes) const
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

bool BaseBvhComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, pragma::bvh::HitInfo &outHitInfo) const
{
	auto bvhData = GetUpdatedBvh();
	if(!bvhData || bvhData->primitives.empty())
		return false;
	bvh::BvhData::HitData bvhHitData;
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
		pragma::bvh::MeshRange search {};
		search.start = bvhHitData.primitiveIndex * 3;
		auto it = std::upper_bound(bvhData->meshRanges.begin(), bvhData->meshRanges.end(), search);
		assert(it != bvhData->meshRanges.begin());
		--it;

		auto distance = minDist + (maxDist - minDist) * (bvhHitData.tmax - bvhHitData.tmin); // TODO: Is this correct?
		auto &hitInfo = outHitInfo;
		hitInfo.primitiveIndex = bvhHitData.primitiveIndex - it->start / 3;
		hitInfo.distance = distance;
		hitInfo.u = bvhHitData.u;
		hitInfo.v = bvhHitData.v;
		hitInfo.t = bvhHitData.tmax;
		hitInfo.mesh = it->mesh;
		hitInfo.entity = it->entity ? it->entity->GetHandle() : GetEntity().GetHandle();
		return true;
	}
	return false;
}

const pragma::bvh::MeshRange *BaseBvhComponent::FindPrimitiveMeshInfo(size_t primIdx) const { return m_bvhData->FindMeshRange(primIdx); }

std::optional<pragma::bvh::HitInfo> BaseBvhComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist) const
{
	pragma::bvh::HitInfo hitInfo {};
	if(IntersectionTest(origin, dir, minDist, maxDist, hitInfo))
		return hitInfo;
	return {};
}

BaseBvhComponent::BaseBvhComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
BaseBvhComponent::~BaseBvhComponent() {}
void BaseBvhComponent::Initialize() { BaseEntityComponent::Initialize(); }
