// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include <bvh/v2/bbox.h>
#include <bvh/v2/bvh.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/thread_pool.h>
#include <bvh/v2/tri.h>
#include <bvh/v2/vec.h>

export module pragma.shared:entities.components.bvh_data;

export import :entities.components.base;
export import :entities.components.intersection_handler;
export import :model;

export namespace pragma::bvh {
	constexpr bool should_permute = true;

	struct DLLNETWORK MeshRange {
		ecs::BaseEntity *entity = nullptr;
		std::shared_ptr<geometry::ModelSubMesh> mesh;
		size_t start;
		size_t end;
		bool operator<(const MeshRange &other) const { return start < other.start; }
	};

	struct DLLNETWORK MeshIntersectionRange {
		MeshIntersectionRange(const MeshRange &range) : entity {range.entity}, mesh {range.mesh.get()}, start {range.start}, end {range.end} {}
		MeshIntersectionRange(geometry::ModelSubMesh &mesh, size_t start, size_t end) : mesh {&mesh}, start {start}, end {end} {}
		ecs::BaseEntity *entity = nullptr;
		geometry::ModelSubMesh *mesh = nullptr;
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
		void Deserialize(const std::vector<uint8_t> &data, std::vector<Primitive> &&primitives);
	  private:
		virtual bool DoInitializeBvh(Executor &executor, ::bvh::v2::DefaultBuilder<Node>::Config &config) override;
		void Refit();
		void InitializePrecomputedTris();
		std::vector<PrecomputedTri> precomputed_tris;
		bool dirty = true;
	};

	DLLNETWORK Primitive create_triangle(const Vector3 &a, const Vector3 &b, const Vector3 &c);
	DLLNETWORK std::vector<MeshRange> &get_bvh_mesh_ranges(MeshBvhTree &bvhData);

	struct DLLNETWORK IntersectionCache {
		std::vector<MeshIntersectionRange> meshRanges;
		std::unordered_set<size_t> meshes;
	};

	DLLNETWORK std::tuple<bool, bool, bool> test_node_aabb_intersection(const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const Node &left, const Node &right);
	DLLNETWORK bool test_bvh_intersection(const MeshBvhTree &bvhData, const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const std::function<bool(const Primitive &)> &testTri, size_t nodeIdx = 0,
	  IntersectionInfo *outIntersectionInfo = nullptr);
	DLLNETWORK bool test_bvh_intersection_with_aabb(const MeshBvhTree &bvhData, const Vector3 &min, const Vector3 &max, size_t nodeIdx = 0, IntersectionInfo *outIntersectionInfo = nullptr);
	DLLNETWORK bool test_bvh_intersection_with_obb(const MeshBvhTree &bvhData, const Vector3 &origin, const Quat &rot, const Vector3 &min, const Vector3 &max, size_t nodeIdx = 0, IntersectionInfo *outIntersectionInfo = nullptr);
	DLLNETWORK bool test_bvh_intersection_with_kdop(const MeshBvhTree &bvhData, const std::vector<math::Plane> &kdop, size_t nodeIdx = 0, IntersectionInfo *outIntersectionInfo = nullptr);

	DLLNETWORK Ray get_ray(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist);
	DLLNETWORK const Vec &to_bvh_vector(const Vector3 &v);
	DLLNETWORK const Vector3 &from_bvh_vector(const Vec &v);
	DLLNETWORK bool is_mesh_bvh_compatible(const geometry::ModelSubMesh &mesh);

	DLLNETWORK std::unordered_map<std::string, std::shared_ptr<geometry::ModelSubMesh>> get_uuid_mesh_map(asset::Model &mdl);

	namespace debug {
		Color DEFAULT_NODE_COLOR = Color {0, 255, 0, 64};
		DLLNETWORK void print_bvh_tree(Bvh &bvh);
		DLLNETWORK void draw_bvh_tree(const Game &game, Bvh &bvh, const math::ScaledTransform &pose = {}, float duration = 20.f);
		DLLNETWORK void draw_node(const Game &game, const Node &node, const math::ScaledTransform &pose = {}, const Color &col = DEFAULT_NODE_COLOR, float duration = 20.f);
		DLLNETWORK void draw_node(const Game &game, const BBox &bbox, const math::ScaledTransform &pose = {}, const Color &col = DEFAULT_NODE_COLOR, float duration = 20.f);
	};
};

export namespace pragma {
	namespace bvh {
		DLLNETWORK std::unique_ptr<MeshBvhTree> create_bvh_data(std::vector<Primitive> &&triangles);
	};
};
