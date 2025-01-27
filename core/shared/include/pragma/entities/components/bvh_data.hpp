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
#include <bvh/v2/default_builder.h>
#include <memory>

namespace pragma {
	struct IntersectionInfo;
};
namespace pragma::bvh {
	constexpr bool should_permute = true;

	struct DLLNETWORK MeshRange {
		BaseEntity *entity = nullptr;
		std::shared_ptr<ModelSubMesh> mesh;
		size_t start;
		size_t end;
		bool operator<(const MeshRange &other) const { return start < other.start; }
	};

	struct DLLNETWORK MeshIntersectionRange {
		MeshIntersectionRange(const MeshRange &range) : entity {range.entity}, mesh {range.mesh.get()}, start {range.start}, end {range.end} {}
		MeshIntersectionRange(ModelSubMesh &mesh, size_t start, size_t end) : mesh {&mesh}, start {start}, end {end} {}
		BaseEntity *entity = nullptr;
		ModelSubMesh *mesh = nullptr;
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
	using Executor = ::bvh::v2::SequentialExecutor; // ::bvh::v2::ParallelExecutor;
	struct DLLNETWORK BvhTree {
		BvhTree();
		virtual ~BvhTree();
		Bvh bvh;

		void InitializeBvh();
		::bvh::v2::ThreadPool &GetThreadPool();
	  protected:
		::bvh::v2::DefaultBuilder<Node>::Config InitializeExecutor();
		std::unique_ptr<Executor> executor {};
	  private:
		virtual bool DoInitializeBvh(Executor &executor, ::bvh::v2::DefaultBuilder<Node>::Config &config) = 0;
	};

	struct DLLNETWORK MeshBvhTree : public BvhTree {
		struct DLLNETWORK HitData {
			size_t primitiveIndex;
			float u;
			float v;
			float t;
		};

		MeshBvhTree() = default;
		bool Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitData &outHitData);
		std::vector<Primitive> primitives;
		std::vector<MeshRange> meshRanges;

		void SetDirty() { dirty = true; }
		bool IsDirty() const { return dirty; }
		void Update();

		const MeshRange *FindMeshRange(size_t primIdx) const;
		void Deserialize(const std::vector<uint8_t> &data, std::vector<pragma::bvh::Primitive> &&primitives);
	  private:
		virtual bool DoInitializeBvh(Executor &executor, ::bvh::v2::DefaultBuilder<Node>::Config &config) override;
		void Refit();
		void InitializePrecomputedTris();
		std::vector<PrecomputedTri> precomputed_tris;
		bool dirty = true;
	};

	DLLNETWORK Primitive create_triangle(const Vector3 &a, const Vector3 &b, const Vector3 &c);
	DLLNETWORK std::vector<bvh::MeshRange> &get_bvh_mesh_ranges(bvh::MeshBvhTree &bvhData);

	struct DLLNETWORK IntersectionCache {
		std::vector<pragma::bvh::MeshIntersectionRange> meshRanges;
		std::unordered_set<size_t> meshes;
	};

	DLLNETWORK std::tuple<bool, bool, bool> test_node_aabb_intersection(const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const pragma::bvh::Node &left, const pragma::bvh::Node &right);
	DLLNETWORK bool test_bvh_intersection(const pragma::bvh::MeshBvhTree &bvhData, const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const std::function<bool(const pragma::bvh::Primitive &)> &testTri, size_t nodeIdx = 0,
	  pragma::IntersectionInfo *outIntersectionInfo = nullptr);
	DLLNETWORK bool test_bvh_intersection_with_aabb(const pragma::bvh::MeshBvhTree &bvhData, const Vector3 &min, const Vector3 &max, size_t nodeIdx = 0, pragma::IntersectionInfo *outIntersectionInfo = nullptr);
	DLLNETWORK bool test_bvh_intersection_with_obb(const pragma::bvh::MeshBvhTree &bvhData, const Vector3 &origin, const Quat &rot, const Vector3 &min, const Vector3 &max, size_t nodeIdx = 0, pragma::IntersectionInfo *outIntersectionInfo = nullptr);
	DLLNETWORK bool test_bvh_intersection_with_kdop(const pragma::bvh::MeshBvhTree &bvhData, const std::vector<umath::Plane> &kdop, size_t nodeIdx = 0, pragma::IntersectionInfo *outIntersectionInfo = nullptr);

	DLLNETWORK Ray get_ray(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist);
	DLLNETWORK const ::pragma::bvh::Vec &to_bvh_vector(const Vector3 &v);
	DLLNETWORK const Vector3 &from_bvh_vector(const ::pragma::bvh::Vec &v);
	DLLNETWORK bool is_mesh_bvh_compatible(const ::ModelSubMesh &mesh);

	DLLNETWORK std::unordered_map<std::string, std::shared_ptr<ModelSubMesh>> get_uuid_mesh_map(Model &mdl);

	namespace debug {
		static Color DEFAULT_NODE_COLOR = Color {0, 255, 0, 64};
		DLLNETWORK void print_bvh_tree(pragma::bvh::Bvh &bvh);
		DLLNETWORK void draw_bvh_tree(const Game &game, pragma::bvh::Bvh &bvh, const umath::ScaledTransform &pose = {}, float duration = 20.f);
		DLLNETWORK void draw_node(const Game &game, const pragma::bvh::Node &node, const umath::ScaledTransform &pose = {}, const Color &col = DEFAULT_NODE_COLOR, float duration = 20.f);
		DLLNETWORK void draw_node(const Game &game, const pragma::bvh::BBox &bbox, const umath::ScaledTransform &pose = {}, const Color &col = DEFAULT_NODE_COLOR, float duration = 20.f);
	};
};

#endif
