/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/bvh_data.hpp"
#include <sharedutils/util_hash.hpp>
#include <mathutil/umath_geometry.hpp>

static ::bvh::Vector3<float> to_bvh_vector(const Vector3 &v) { return {v.x, v.y, v.z}; }

pragma::bvh::MeshIntersectionInfo *pragma::bvh::IntersectionInfo::GetMeshIntersectionInfo() { return m_isMeshIntersectionInfo ? static_cast<pragma::bvh::MeshIntersectionInfo *>(this) : nullptr; }

pragma::bvh::Primitive pragma::bvh::create_triangle(const Vector3 &a, const Vector3 &b, const Vector3 &c) { return pragma::bvh::Primitive {to_bvh_vector(a), to_bvh_vector(b), to_bvh_vector(c)}; }

std::vector<pragma::bvh::MeshRange> &pragma::bvh::get_bvh_mesh_ranges(pragma::bvh::BvhData &bvhData) { return bvhData.meshRanges; }

void pragma::bvh::IntersectionInfo::Clear() { primitives.clear(); }

static void get_bvh_bounds(const bvh::BoundingBox<float> &bb, Vector3 &outMin, Vector3 &outMax)
{
	constexpr auto epsilon = 0.001f;
	outMin = Vector3 {umath::min(bb.min.values[0], bb.max.values[0]) - epsilon, umath::min(bb.min.values[1], bb.max.values[1]) - epsilon, umath::min(bb.min.values[2], bb.max.values[2]) - epsilon};
	outMax = Vector3 {umath::max(bb.min.values[0], bb.max.values[0]) + epsilon, umath::max(bb.min.values[1], bb.max.values[1]) + epsilon, umath::max(bb.min.values[2], bb.max.values[2]) + epsilon};
}
bool pragma::bvh::test_bvh_intersection(const pragma::bvh::BvhData &bvhData, const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const std::function<bool(const pragma::bvh::Primitive &)> &testTri, size_t nodeIdx, pragma::bvh::IntersectionInfo *outIntersectionInfo)
{
	auto &bvh = bvhData.bvh;
	auto &node = bvh.nodes.get()[nodeIdx];
	Vector3 v0, v1;
	get_bvh_bounds(node.bounding_box_proxy().to_bounding_box(), v0, v1);
	auto intersect = testAabb(v0, v1);
	if(!intersect)
		return false;
	if(node.is_leaf()) {
		auto hasHit = false;
		auto *meshIntersectionInfo = outIntersectionInfo ? outIntersectionInfo->GetMeshIntersectionInfo() : nullptr;
		for(auto i = decltype(node.primitive_count) {0u}; i < node.primitive_count; ++i) {
			auto primIdx = bvh.primitive_indices[node.first_child_or_primitive + i];

			if(meshIntersectionInfo) {
				auto skip = false;
				auto idx = primIdx * 3;
				for(auto *meshRange : meshIntersectionInfo->GetTemporaryMeshRanges()) {
					if(idx >= meshRange->start && idx < meshRange->end) {
						skip = true;
						break;
					}
				}
				if(skip)
					continue;
			}

			auto &prim = bvhData.primitives[primIdx];
			auto p1 = prim.p1();
			auto p2 = prim.p2();
			auto res = testTri(prim);
			if(res) {
				if(!outIntersectionInfo)
					return true;
				auto addPrim = true;
				if(meshIntersectionInfo) {
					auto *meshRange = bvhData.FindMeshRange(primIdx);
					assert(meshRange != nullptr);
					meshIntersectionInfo->GetTemporaryMeshRanges().push_back(meshRange);

					auto &tmpMeshes = meshIntersectionInfo->GetTemporarMeshMap();
					auto hash = util::hash_combine<uint64_t>(util::hash_combine<uint64_t>(0, reinterpret_cast<uint64_t>(meshRange->mesh.get())), reinterpret_cast<uint64_t>(meshRange->entity));
					auto it = tmpMeshes.find(hash);
					if(it != tmpMeshes.end())
						addPrim = false;
					else
						tmpMeshes.insert(hash);
				}
				if(addPrim) {
					if(outIntersectionInfo->primitives.size() == outIntersectionInfo->primitives.capacity())
						outIntersectionInfo->primitives.reserve(outIntersectionInfo->primitives.size() * 1.75);
					outIntersectionInfo->primitives.push_back(primIdx);
				}
				hasHit = true;
			}
		}
		if(meshIntersectionInfo)
			meshIntersectionInfo->GetTemporaryMeshRanges().clear();
		return hasHit;
	}
	if(!outIntersectionInfo) {
		return test_bvh_intersection(bvhData, testAabb, testTri, node.first_child_or_primitive, outIntersectionInfo) || test_bvh_intersection(bvhData, testAabb, testTri, node.first_child_or_primitive + 1, outIntersectionInfo);
	}
	// We have to visit both
	auto a = test_bvh_intersection(bvhData, testAabb, testTri, node.first_child_or_primitive, outIntersectionInfo);
	auto b = test_bvh_intersection(bvhData, testAabb, testTri, node.first_child_or_primitive + 1, outIntersectionInfo);
	return a || b;
}
bool pragma::bvh::test_bvh_intersection_with_aabb(const pragma::bvh::BvhData &bvhData, const Vector3 &min, const Vector3 &max, size_t nodeIdx, pragma::bvh::IntersectionInfo *outIntersectionInfo)
{
	return test_bvh_intersection(
	  bvhData, [&min, &max](const Vector3 &aabbMin, const Vector3 &aabbMax) -> bool { return umath::intersection::aabb_aabb(min, max, aabbMin, aabbMax) != umath::intersection::Intersect::Outside; },
	  [&min, &max](const pragma::bvh::Primitive &prim) -> bool {
		  auto p1 = prim.p1();
		  auto p2 = prim.p2();
		  return umath::intersection::aabb_triangle(min, max, *reinterpret_cast<const Vector3 *>(&prim.p0.values), *reinterpret_cast<const Vector3 *>(&p1.values), *reinterpret_cast<const Vector3 *>(&p2.values));
	  },
	  nodeIdx, outIntersectionInfo);
}
bool pragma::bvh::test_bvh_intersection_with_kdop(const pragma::bvh::BvhData &bvhData, const std::vector<umath::Plane> &kdop, size_t nodeIdx, pragma::bvh::IntersectionInfo *outIntersectionInfo)
{
	return test_bvh_intersection(
	  bvhData, [&kdop](const Vector3 &aabbMin, const Vector3 &aabbMax) -> bool { return umath::intersection::aabb_in_plane_mesh(aabbMin, aabbMax, kdop) != umath::intersection::Intersect::Outside; },
	  [&kdop](const pragma::bvh::Primitive &prim) -> bool {
		  Vector3 v0, v1;
		  get_bvh_bounds(prim.bounding_box(), v0, v1);
		  return umath::intersection::aabb_in_plane_mesh(v0, v1, kdop) != umath::intersection::Intersect::Outside;
	  },
	  nodeIdx, outIntersectionInfo);
}

pragma::bvh::BvhData::BvhData() {}

pragma::bvh::BvhData::IntersectorData::IntersectorData(::bvh::SweepSahBuilder<::bvh::Bvh<float>> builder, ::bvh::ClosestPrimitiveIntersector<::bvh::Bvh<float>, ::bvh::Triangle<float>> primitiveIntersector, ::bvh::SingleRayTraverser<::bvh::Bvh<float>> traverser)
    : builder {std::move(builder)}, primitiveIntersector {std::move(primitiveIntersector)}, traverser {std::move(traverser)}
{
}

const pragma::bvh::MeshRange *pragma::bvh::BvhData::FindMeshRange(size_t primIdx) const
{
	MeshRange search {};
	search.start = primIdx * 3;
	auto it = std::upper_bound(meshRanges.begin(), meshRanges.end(), search);
	if(it == meshRanges.begin())
		return nullptr;
	--it;
	return &*it;
}

void pragma::bvh::BvhData::InitializeIntersectorData()
{
	auto [bboxes, centers] = ::bvh::compute_bounding_boxes_and_centers(primitives.data(), primitives.size());
	auto global_bbox = ::bvh::compute_bounding_boxes_union(bboxes.get(), primitives.size());

	::bvh::SweepSahBuilder<::bvh::Bvh<float>> builder {bvh};
	builder.build(global_bbox, bboxes.get(), centers.get(), primitives.size());
	::bvh::ClosestPrimitiveIntersector<::bvh::Bvh<float>, ::bvh::Triangle<float>> primitive_intersector(bvh, primitives.data());
	::bvh::SingleRayTraverser<::bvh::Bvh<float>> traverser {bvh};
	intersectorData = std::make_unique<IntersectorData>(std::move(builder), std::move(primitive_intersector), std::move(traverser));
}

::bvh::Ray<float> pragma::bvh::get_ray(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist) { return {::bvh::Vector3<float>(origin.x, origin.y, origin.z), ::bvh::Vector3<float>(dir.x, dir.y, dir.z), minDist, maxDist}; }
