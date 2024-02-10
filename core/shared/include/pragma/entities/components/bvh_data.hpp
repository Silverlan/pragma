/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#ifndef __BVH_DATA_HPP__
#define __BVH_DATA_HPP__

#include "pragma/networkdefinitions.h"
#include <bvh/v2/bvh.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/tri.h>
#include <bvh/v2/bbox.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/thread_pool.h>
#include <memory>

namespace pragma::bvh {
	struct DLLNETWORK HitInfo {
		std::shared_ptr<ModelSubMesh> mesh;
		EntityHandle entity;
		size_t primitiveIndex;
		float distance;
		float t;
		float u;
		float v;
	};

	struct MeshIntersectionInfo;
	struct DLLNETWORK IntersectionInfo {
		void Clear();
		std::vector<size_t> primitives;

		MeshIntersectionInfo *GetMeshIntersectionInfo();
	  protected:
		bool m_isMeshIntersectionInfo = false;
	};

	struct MeshRange;
	struct DLLNETWORK MeshIntersectionInfo : public IntersectionInfo {
		MeshIntersectionInfo() { m_isMeshIntersectionInfo = true; }

		// For internal use only
		std::vector<const pragma::bvh::MeshRange *> &GetTemporaryMeshRanges() { return m_tmpMeshRanges; }
		std::unordered_set<size_t> &GetTemporarMeshMap() { return m_tmpMeshes; }
	  protected:
		std::vector<const pragma::bvh::MeshRange *> m_tmpMeshRanges;
		std::unordered_set<size_t> m_tmpMeshes;
	};

	struct DLLNETWORK MeshRange {
		BaseEntity *entity = nullptr;
		std::shared_ptr<ModelSubMesh> mesh;
		size_t start;
		size_t end;
		bool operator<(const MeshRange &other) const { return start < other.start; }
	};

	using Scalar = float;
	using Vec = ::bvh::v2::Vec<Scalar, 3>;
	using BBox = ::bvh::v2::BBox<Scalar, 3>;
	using Primitive = ::bvh::v2::Tri<Scalar, 3>;
	using Node = ::bvh::v2::Node<Scalar, 3>;
	using Bvh = ::bvh::v2::Bvh<Node>;
	using Ray = ::bvh::v2::Ray<Scalar, 3>;
	using PrecomputedTri = ::bvh::v2::PrecomputedTri<Scalar>;
	struct DLLNETWORK BvhData {
		struct HitData {
			size_t primitiveIndex;
			float u;
			float v;
			float tmin;
			float tmax;
		};
		BvhData();
		Bvh bvh;
		std::vector<Primitive> primitives;
		std::vector<MeshRange> meshRanges;

		const MeshRange *FindMeshRange(size_t primIdx) const;
		bool Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitData &outHitData);
		void Refit();
		void InitializeBvh();
	  private:
		::bvh::v2::ThreadPool thread_pool;
		std::unique_ptr<::bvh::v2::ParallelExecutor> executor {};
		std::vector<PrecomputedTri> precomputed_tris;
	};

	DLLNETWORK Primitive create_triangle(const Vector3 &a, const Vector3 &b, const Vector3 &c);
	DLLNETWORK std::vector<bvh::MeshRange> &get_bvh_mesh_ranges(bvh::BvhData &bvhData);
	DLLNETWORK bool test_bvh_intersection(const pragma::bvh::BvhData &bvhData, const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const std::function<bool(const pragma::bvh::Primitive &)> &testTri, size_t nodeIdx = 0,
	  pragma::bvh::IntersectionInfo *outIntersectionInfo = nullptr);
	DLLNETWORK bool test_bvh_intersection_with_aabb(const pragma::bvh::BvhData &bvhData, const Vector3 &min, const Vector3 &max, size_t nodeIdx = 0, pragma::bvh::IntersectionInfo *outIntersectionInfo = nullptr);
	DLLNETWORK bool test_bvh_intersection_with_kdop(const pragma::bvh::BvhData &bvhData, const std::vector<umath::Plane> &kdop, size_t nodeIdx = 0, pragma::bvh::IntersectionInfo *outIntersectionInfo = nullptr);

	DLLNETWORK Ray get_ray(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist);
};

#endif
