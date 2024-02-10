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
#include <bvh/v2/default_builder.h>
#include <bvh/v2/stack.h>
#include <bvh/v2/reinsertion_optimizer.h>

static_assert(sizeof(Vector3) == sizeof(::pragma::bvh::Vec));
static const ::pragma::bvh::Vec &to_bvh_vector(const Vector3 &v) { return reinterpret_cast<const ::pragma::bvh::Vec &>(v); }

pragma::bvh::MeshIntersectionInfo *pragma::bvh::IntersectionInfo::GetMeshIntersectionInfo() { return m_isMeshIntersectionInfo ? static_cast<pragma::bvh::MeshIntersectionInfo *>(this) : nullptr; }

pragma::bvh::Primitive pragma::bvh::create_triangle(const Vector3 &a, const Vector3 &b, const Vector3 &c) { return pragma::bvh::Primitive {to_bvh_vector(a), to_bvh_vector(b), to_bvh_vector(c)}; }

std::vector<pragma::bvh::MeshRange> &pragma::bvh::get_bvh_mesh_ranges(pragma::bvh::BvhData &bvhData) { return bvhData.meshRanges; }

void pragma::bvh::IntersectionInfo::Clear() { primitives.clear(); }

static void get_bvh_bounds(const pragma::bvh::BBox &bb, Vector3 &outMin, Vector3 &outMax)
{
	constexpr auto epsilon = 0.001f;
	outMin = Vector3 {umath::min(bb.min.values[0], bb.max.values[0]) - epsilon, umath::min(bb.min.values[1], bb.max.values[1]) - epsilon, umath::min(bb.min.values[2], bb.max.values[2]) - epsilon};
	outMax = Vector3 {umath::max(bb.min.values[0], bb.max.values[0]) + epsilon, umath::max(bb.min.values[1], bb.max.values[1]) + epsilon, umath::max(bb.min.values[2], bb.max.values[2]) + epsilon};
}
bool pragma::bvh::test_bvh_intersection(const pragma::bvh::BvhData &bvhData, const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const std::function<bool(const pragma::bvh::Primitive &)> &testTri, size_t nodeIdx, pragma::bvh::IntersectionInfo *outIntersectionInfo)
{
	auto &bvh = bvhData.bvh;
	auto &node = bvh.nodes[nodeIdx];
	constexpr size_t stack_size = 64;
	::bvh::v2::SmallStack<bvh::Bvh::Index, stack_size> stack;
	auto hasAnyHit = false;
	auto *meshIntersectionInfo = outIntersectionInfo ? outIntersectionInfo->GetMeshIntersectionInfo() : nullptr;
	auto traverse = [&]<bool ReturnOnFirstHit>() {
		bvh.traverse<ReturnOnFirstHit>(
		  bvh.get_root().index, stack,
		  [meshIntersectionInfo, outIntersectionInfo, &bvhData, &testTri, &hasAnyHit](size_t begin, size_t end) {
			  auto hasHit = false;
			  for(auto i = begin; i < end; ++i) {
				  auto primIdx = i;

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
				  auto &p1 = prim.p1;
				  auto &p2 = prim.p2;
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
					  hasAnyHit = true;
				  }
			  }

			  if(meshIntersectionInfo)
				  meshIntersectionInfo->GetTemporaryMeshRanges().clear();
			  return hasHit;
		  },
		  [&testAabb](const Node &left, const Node &right, bool &hit_left, bool &hit_right) {
			  {
				  Vector3 v0_l, v1_l;
				  get_bvh_bounds(left.get_bbox(), v0_l, v1_l);
				  hit_left = testAabb(v0_l, v1_l);
			  }

			  {
				  Vector3 v0_r, v1_r;
				  get_bvh_bounds(right.get_bbox(), v0_r, v1_r);
				  hit_right = testAabb(v0_r, v1_r);
			  }
		  });
	};
	if(!outIntersectionInfo)
		traverse.template operator()<true>();
	else
		traverse.template operator()<false>();
	return hasAnyHit;
}
bool pragma::bvh::test_bvh_intersection_with_aabb(const pragma::bvh::BvhData &bvhData, const Vector3 &min, const Vector3 &max, size_t nodeIdx, pragma::bvh::IntersectionInfo *outIntersectionInfo)
{
	return test_bvh_intersection(
	  bvhData, [&min, &max](const Vector3 &aabbMin, const Vector3 &aabbMax) -> bool { return umath::intersection::aabb_aabb(min, max, aabbMin, aabbMax) != umath::intersection::Intersect::Outside; },
	  [&min, &max](const pragma::bvh::Primitive &prim) -> bool { return umath::intersection::aabb_triangle(min, max, *reinterpret_cast<const Vector3 *>(&prim.p0.values), *reinterpret_cast<const Vector3 *>(&prim.p1.values), *reinterpret_cast<const Vector3 *>(&prim.p2.values)); }, nodeIdx,
	  outIntersectionInfo);
}
bool pragma::bvh::test_bvh_intersection_with_kdop(const pragma::bvh::BvhData &bvhData, const std::vector<umath::Plane> &kdop, size_t nodeIdx, pragma::bvh::IntersectionInfo *outIntersectionInfo)
{
	return test_bvh_intersection(
	  bvhData, [&kdop](const Vector3 &aabbMin, const Vector3 &aabbMax) -> bool { return umath::intersection::aabb_in_plane_mesh(aabbMin, aabbMax, kdop) != umath::intersection::Intersect::Outside; },
	  [&kdop](const pragma::bvh::Primitive &prim) -> bool {
		  Vector3 v0, v1;
		  get_bvh_bounds(prim.get_bbox(), v0, v1);
		  return umath::intersection::aabb_in_plane_mesh(v0, v1, kdop) != umath::intersection::Intersect::Outside;
	  },
	  nodeIdx, outIntersectionInfo);
}

pragma::bvh::BvhData::BvhData() {}
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

constexpr bool should_permute = true;
bool pragma::bvh::BvhData::Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitData &outHitData)
{
	constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
	constexpr size_t stack_size = 64;
	constexpr bool use_robust_traversal = false;

	auto &prim_id = outHitData.primitiveIndex = invalid_id;
	auto &u = outHitData.u;
	auto &v = outHitData.v;
	::bvh::v2::SmallStack<bvh::Bvh::Index, stack_size> stack;
	auto ray = get_ray(origin, dir, minDist, maxDist);
	bvh.intersect<false, use_robust_traversal>(ray, bvh.get_root().index, stack, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			size_t j = should_permute ? i : bvh.prim_ids[i];
			if(auto hit = precomputed_tris[j].intersect(ray)) {
				prim_id = i;
				std::tie(u, v) = *hit;
			}
		}
		return prim_id != invalid_id;
	});
	outHitData.tmin = ray.tmin;
	outHitData.tmax = ray.tmax;
	return prim_id != invalid_id;
}

void pragma::bvh::BvhData::Refit() { ::bvh::v2::ReinsertionOptimizer<Node>::optimize(thread_pool, bvh); }

void pragma::bvh::BvhData::InitializeBvh()
{
	executor = std::make_unique<::bvh::v2::ParallelExecutor>(thread_pool);

	auto numTris = primitives.size();
	std::vector<pragma::bvh::BBox> bboxes {numTris};
	std::vector<Vec> centers {numTris};
	executor->for_each(0, numTris, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			bboxes[i] = primitives[i].get_bbox();
			centers[i] = primitives[i].get_center();
		}
	});

	::bvh::v2::DefaultBuilder<Node>::Config config;
	config.quality = ::bvh::v2::DefaultBuilder<Node>::Quality::High;
	bvh = ::bvh::v2::DefaultBuilder<Node>::build(thread_pool, bboxes, centers, config);

	precomputed_tris.resize(numTris);
	executor->for_each(0, numTris, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			auto j = should_permute ? bvh.prim_ids[i] : i;
			precomputed_tris[i] = primitives[j];
		}
	});
}

pragma::bvh::Ray pragma::bvh::get_ray(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist) { return Ray {to_bvh_vector(origin), to_bvh_vector(dir), minDist, maxDist}; }
