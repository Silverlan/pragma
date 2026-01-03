// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"
#include <bvh/v2/default_builder.h>

export module pragma.client:entities.components.hitbox_bvh;

export import :model.hitbox_mesh_bvh_builder;

export namespace pragma {
	namespace bvh {
		class ObbBvhTree;
		struct DLLCLIENT DebugDrawInfo {
			enum class Flags : uint8_t {
				None = 0u,
				DrawTraversedNodesBit = 1u,
				DrawTraversedLeavesBit = DrawTraversedNodesBit << 1u,
				DrawHitLeavesBit = DrawTraversedLeavesBit << 1u,
				DrawTraversedMeshesBit = DrawHitLeavesBit << 1u,
				DrawHitMeshesBit = DrawTraversedMeshesBit << 1u,
			};
			Flags flags = Flags::DrawHitLeavesBit;
			math::ScaledTransform basePose;
			float duration = 0.1f;
		};
		struct DLLCLIENT MeshHitboxBvhCache {
			using TriangleIndex = uint32_t;
			std::shared_ptr<MeshBvhTree> bvhTree;
			std::shared_ptr<geometry::ModelSubMesh> mesh;
			std::vector<TriangleIndex> bvhTriToOriginalTri;
		};
		struct DLLCLIENT BoneHitboxBvhCache {
			using Uuid = std::string;
			std::unordered_map<Uuid, std::shared_ptr<MeshHitboxBvhCache>> meshCache;
		};
		struct DLLCLIENT ModelHitboxBvhCache {
			std::shared_future<void> task;
			std::atomic<bool> complete = false;

			std::unordered_map<animation::BoneId, std::shared_ptr<BoneHitboxBvhCache>> boneCache;
		};
		struct DLLCLIENT HitboxBvhCache {
			using ModelName = std::string;
			HitboxBvhCache(Game &game);
			~HitboxBvhCache();
			ModelHitboxBvhCache *GetModelCache(const ModelName &mdlName);
			std::shared_future<void> GenerateModelCache(const ModelName &mdlName, asset::Model &mdl);
		  private:
			void PrepareModel(asset::Model &mdl);
			void InitializeModelHitboxBvhCache(asset::Model &mdl, const HitboxMeshBvhBuildTask &buildTask, ModelHitboxBvhCache &mdlHbBvhCache);
			std::unordered_map<ModelName, std::shared_ptr<ModelHitboxBvhCache>> m_modelBvhCache;
			Game &m_game;
			HitboxMeshBvhBuilder m_builder;
		};
	};

	class DLLCLIENT CHitboxBvhComponent final : public BaseEntityComponent {
	  public:
		CHitboxBvhComponent(ecs::BaseEntity &ent);
		virtual ~CHitboxBvhComponent() override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		void InitializeBvh();
		bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo, const bvh::DebugDrawInfo *debugDrawInfo = nullptr) const;
		bool IntersectionTestAabb(const Vector3 &min, const Vector3 &max, IntersectionInfo *outIntersectionInfo = nullptr) const;
		bool IntersectionTestKDop(const std::vector<math::Plane> &planes, IntersectionInfo *outIntersectionInfo = nullptr) const;
		void DebugDrawHitboxMeshes(animation::BoneId boneId, float duration = 12.f) const;
		bvh::HitboxBvhCache &GetGlobalBvhCache() const;
	  private:
		void Reset();
		bool UpdateHitboxMeshCache() const;
		void WaitForHitboxBvhUpdate();
		void DebugDraw();
		void OnModelChanged();
		void InitializeHitboxBvh();
		bool InitializeModel();
		void InitializeHitboxMeshBvhs();
		void UpdateHitboxBvh();
		std::shared_future<void> m_hitboxMeshCacheTask;
		std::unordered_map<animation::BoneId, std::vector<std::shared_ptr<bvh::MeshHitboxBvhCache>>> m_hitboxMeshBvhCaches;
		std::shared_ptr<bvh::ObbBvhTree> m_hitboxBvh;

		std::vector<math::ScaledTransform> m_hitboxBvhUpdatePoses;
		std::future<void> m_hitboxBvhUpdate;
	};

	namespace bvh {
		struct DLLCLIENT HitboxObb {
			HitboxObb(const Vector3 &min, const Vector3 &max);
			BBox ToBvhBBox(const math::ScaledTransform &pose, Vector3 &outOrigin) const;
			math::ScaledTransform GetPose(const std::vector<math::ScaledTransform> &effectivePoses) const;
			Vector3 position; // Position relative to bone
			Vector3 halfExtents;
			Vector3 min;
			Vector3 max;

			animation::BoneId boneId = std::numeric_limits<animation::BoneId>::max();
		};

		struct DLLCLIENT ObbBvhTree : public BvhTree {
			struct DLLCLIENT HitData {
				size_t primitiveIndex;
				float t;
			};

			void InitializeBvh(const std::vector<math::ScaledTransform> &poses);
			bool Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, const std::vector<math::ScaledTransform> &bonePoses, std::vector<HitData> &outHits, const DebugDrawInfo *debugDrawInfo = nullptr);
			std::vector<HitboxObb> primitives;
		  private:
			const std::vector<math::ScaledTransform> *m_poses = nullptr;
			virtual bool DoInitializeBvh(Executor &executor, ::bvh::v2::DefaultBuilder<Node>::Config &config) override;
		};

		DLLCLIENT bool test_bvh_intersection(const ObbBvhTree &bvhData, const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const std::function<bool(const HitboxObb &)> &testObb, size_t nodeIdx = 0, IntersectionInfo *outIntersectionInfo = nullptr);
		DLLCLIENT bool test_bvh_intersection_with_aabb(const ObbBvhTree &bvhData, const std::vector<math::ScaledTransform> &effectivePoses, const Vector3 &min, const Vector3 &max, size_t nodeIdx = 0, IntersectionInfo *outIntersectionInfo = nullptr);
		DLLCLIENT bool test_bvh_intersection_with_obb(const ObbBvhTree &bvhData, const std::vector<math::ScaledTransform> &effectivePoses, const Vector3 &origin, const Quat &rot, const Vector3 &min, const Vector3 &max, size_t nodeIdx = 0, IntersectionInfo *outIntersectionInfo = nullptr);
		DLLCLIENT bool test_bvh_intersection_with_kdop(const ObbBvhTree &bvhData, const std::vector<math::ScaledTransform> &effectivePoses, const std::vector<math::Plane> &kdop, size_t nodeIdx = 0, IntersectionInfo *outIntersectionInfo = nullptr);
	};
};
export {
	REGISTER_ENUM_FLAGS(pragma::bvh::DebugDrawInfo::Flags)
}
