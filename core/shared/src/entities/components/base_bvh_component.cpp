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
#include <bvh/bvh.hpp>
#include <bvh/triangle.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>
#include <bvh/hierarchy_refitter.hpp>

using namespace pragma;

using Primitive = bvh::Triangle<float>;
static_assert(sizeof(BvhTriangle) == sizeof(Primitive));
namespace pragma
{
	struct BvhData
	{
		struct IntersectorData
		{
			bvh::SweepSahBuilder<bvh::Bvh<float>> builder;
			bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, bvh::Triangle<float>> primitiveIntersector;
			bvh::SingleRayTraverser<bvh::Bvh<float>> traverser;
		};
		BvhData();
		bvh::Bvh<float> bvh;
		std::vector<Primitive> primitives;
		std::vector<BvhMeshRange> meshRanges;

		void InitializeIntersectorData();
		std::unique_ptr<IntersectorData> intersectorData;
	};
};

std::vector<BvhMeshRange> &pragma::get_bvh_mesh_ranges(BvhData &bvhData) {return bvhData.meshRanges;}

pragma::BvhData::BvhData()
{}

void pragma::BvhData::InitializeIntersectorData()
{
	auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(primitives.data(),primitives.size());
	auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(),primitives.size());

	bvh::SweepSahBuilder<bvh::Bvh<float>> builder {bvh};
	builder.build(global_bbox,bboxes.get(),centers.get(),primitives.size());
	bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>,bvh::Triangle<float>> primitive_intersector(bvh,primitives.data());
	bvh::SingleRayTraverser<bvh::Bvh<float>> traverser {bvh};
	intersectorData = std::make_unique<IntersectorData>(
		std::move(builder),
		std::move(primitive_intersector),
		std::move(traverser)
	);
}

ComponentEventId BaseBvhComponent::EVENT_ON_CLEAR_BVH = INVALID_COMPONENT_ID;
ComponentEventId BaseBvhComponent::EVENT_ON_BVH_REBUILT = INVALID_COMPONENT_ID;
void BaseBvhComponent::RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent)
{
	EVENT_ON_CLEAR_BVH = registerEvent("EVENT_ON_CLEAR_BVH",EntityComponentManager::EventInfo::Type::Explicit);
	EVENT_ON_BVH_REBUILT = registerEvent("EVENT_ON_BVH_REBUILT",EntityComponentManager::EventInfo::Type::Explicit);
}

void BaseBvhComponent::ClearBvh()
{
	InvokeEventCallbacks(EVENT_ON_CLEAR_BVH);
	m_bvhDataMutex.lock();
		m_bvhData = nullptr;
	m_bvhDataMutex.unlock();
}

void BaseBvhComponent::RebuildBvh()
{
	ClearBvh();
	if(m_staticCache.valid())
	{
		m_staticCache->SetCacheDirty();
		return;
	}
	DoRebuildBvh();

	InvokeEventCallbacks(EVENT_ON_BVH_REBUILT);
}

void BaseBvhComponent::SetStaticCache(BaseStaticBvhCacheComponent *staticCache)
{
	m_staticCache = staticCache ? staticCache->GetHandle<BaseStaticBvhCacheComponent>() : ComponentHandle<BaseStaticBvhCacheComponent>{};
}

bool BaseBvhComponent::SetVertexData(const std::vector<BvhTriangle> &data)
{
	std::scoped_lock lock {m_bvhDataMutex};
	if(m_bvhData->primitives.size() != data.size())
		return false;
	memcpy(m_bvhData->primitives.data(),data.data(),util::size_of_container(data));
	
	// Update bounding boxes
	bvh::HierarchyRefitter<bvh::Bvh<float>> refitter {m_bvhData->bvh};
	refitter.refit([&] (bvh::Bvh<float>::Node& leaf) {
		assert(leaf.is_leaf());
		auto bbox = bvh::BoundingBox<float>::empty();
		for (size_t i = 0; i < leaf.primitive_count; ++i) {
			auto& triangle = m_bvhData->primitives[m_bvhData->bvh.primitive_indices[leaf.first_child_or_primitive + i]];
			bbox.extend(triangle.bounding_box());
		}
		leaf.bounding_box_proxy() = bbox;
	});
	return true;
}

std::shared_ptr<pragma::BvhData> BaseBvhComponent::RebuildBvh(
	const std::vector<std::shared_ptr<ModelSubMesh>> &meshes,const std::vector<umath::ScaledTransform> *optPoses,
	const std::function<bool()> &fIsCancelled,std::vector<size_t> *optOutMeshIndices
)
{
	auto bvhData = std::make_unique<pragma::BvhData>();

	auto shouldUseMesh = [](const ModelSubMesh &mesh) {
		return mesh.GetGeometryType() == ModelSubMesh::GeometryType::Triangles;
	};
	size_t numVerts = 0;
	bvhData->meshRanges.reserve(meshes.size());
	if(optOutMeshIndices)
		optOutMeshIndices->reserve(meshes.size());
	size_t primitiveOffset = 0;
	for(uint32_t meshIdx=0;auto &mesh : meshes)
	{
		if(fIsCancelled && fIsCancelled())
		{
			++meshIdx;
			return nullptr;
		}
		if(shouldUseMesh(*mesh) == false)
		{
			++meshIdx;
			continue;
		}
		bvhData->meshRanges.push_back({});

		auto &rangeInfo = bvhData->meshRanges.back();
		rangeInfo.mesh = mesh;
		rangeInfo.start = primitiveOffset;
		rangeInfo.end = rangeInfo.start +mesh->GetIndexCount();

		numVerts += mesh->GetIndexCount();
		primitiveOffset += mesh->GetIndexCount();
		if(optOutMeshIndices)
			optOutMeshIndices->push_back(meshIdx);
		++meshIdx;
	}
	
	auto &primitives = bvhData->primitives;
	primitives.resize(numVerts /3);
	primitiveOffset = 0;
	for(uint32_t meshIdx=0;auto &mesh : meshes)
	{
		if(fIsCancelled && fIsCancelled())
			return nullptr;
		if(shouldUseMesh(*mesh) == false)
		{
			++meshIdx;
			continue;
		}
		auto *pose = optPoses ? &(*optPoses)[meshIdx] : nullptr;
		auto &verts = mesh->GetVertices();
		mesh->VisitIndices([&verts,primitiveOffset,&primitives,pose](auto *indexDataSrc,uint32_t numIndicesSrc) {
			for(auto i=decltype(numIndicesSrc){0};i<numIndicesSrc;i+=3)
			{
				auto va = verts[indexDataSrc[i]].position;
				auto vb = verts[indexDataSrc[i +1]].position;
				auto vc = verts[indexDataSrc[i +2]].position;

				if(pose)
				{
					va = *pose *va;
					vb = *pose *vb;
					vc = *pose *vc;
				}

				auto &prim = primitives[(primitiveOffset +i) /3];
				prim = {
					{va.x,va.y,va.z},
					{vb.x,vb.y,vb.z},
					{vc.x,vc.y,vc.z}
				};
			}
		});
		primitiveOffset += mesh->GetIndexCount();
		++meshIdx;
	}
	
	bvhData->InitializeIntersectorData();
	return std::move(bvhData);
}

std::vector<BvhMeshRange> &BaseBvhComponent::GetMeshRanges() {return m_bvhData->meshRanges;}

bool BaseBvhComponent::IntersectionTest(
	const Vector3 &origin,const Vector3 &dir,float minDist,float maxDist,
	BvhHitInfo &outHitInfo
) const
{
	if(!m_bvhData)
		return false;
	auto &traverser = m_bvhData->intersectorData->traverser;
	auto &primitiveIntersector = m_bvhData->intersectorData->primitiveIntersector;
	bvh::Ray<float> ray {
		bvh::Vector3<float>(origin.x,origin.y,origin.z), // origin
		bvh::Vector3<float>(dir.x,dir.y,dir.z), // direction
		minDist, // minimum distance
		maxDist // maximum distance
	};
	m_bvhDataMutex.lock();
	auto hit = traverser.traverse(ray, primitiveIntersector);
	m_bvhDataMutex.unlock();
	if(hit)
	{
		BvhMeshRange search {};
		search.start = hit->primitive_index *3;
		auto it = std::upper_bound(m_bvhData->meshRanges.begin(),m_bvhData->meshRanges.end(),search);
		assert(it != m_bvhData->meshRanges.begin());
		--it;

		auto &hitInfo = outHitInfo;
		hitInfo.primitiveIndex = hit->primitive_index -it->start;
		hitInfo.distance = hit->distance();
		hitInfo.u = hit->intersection.u;
		hitInfo.v = hit->intersection.v;
		hitInfo.t = hit->intersection.t;
		hitInfo.mesh = it->mesh;
		hitInfo.entity = it->entity ? it->entity->GetHandle() : GetEntity().GetHandle();
		return true;
	}
	return false;
}

std::optional<pragma::BvhHitInfo> BaseBvhComponent::IntersectionTest(
	const Vector3 &origin,const Vector3 &dir,float minDist,float maxDist
) const
{
	pragma::BvhHitInfo hitInfo {};
	if(IntersectionTest(origin,dir,minDist,maxDist,hitInfo))
		return hitInfo;
	return {};
}

BaseBvhComponent::BaseBvhComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
BaseBvhComponent::~BaseBvhComponent() {}
void BaseBvhComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}
