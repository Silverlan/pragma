/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __C_HITBOX_BVH_COMPONENT_HPP__
#define __C_HITBOX_BVH_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_bvh_component.hpp>
#include "pragma/entities/components/hitbox_mesh_bvh_builder.hpp"

class Model;
class ModelSubMesh;
namespace pragma {
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
			umath::ScaledTransform basePose;
			float duration = 0.1f;
		};
		struct DLLCLIENT MeshHitboxBvhCache {
			using TriangleIndex = uint32_t;
			std::shared_ptr<pragma::bvh::MeshBvhTree> bvhTree;
			std::shared_ptr<::ModelSubMesh> mesh;
			std::vector<TriangleIndex> bvhTriToOriginalTri;
		};
		struct DLLCLIENT BoneHitboxBvhCache {
			using Uuid = std::string;
			std::unordered_map<Uuid, std::shared_ptr<MeshHitboxBvhCache>> meshCache;
		};
		struct DLLCLIENT ModelHitboxBvhCache {
			std::shared_future<void> task;
			std::atomic<bool> complete = false;

			std::unordered_map<pragma::animation::BoneId, std::shared_ptr<BoneHitboxBvhCache>> boneCache;
		};
		struct DLLCLIENT HitboxBvhCache {
			using ModelName = std::string;
			HitboxBvhCache(Game &game);
			~HitboxBvhCache();
			ModelHitboxBvhCache *GetModelCache(const ModelName &mdlName);
			std::shared_future<void> GenerateModelCache(const ModelName &mdlName, Model &mdl);
		  private:
			void PrepareModel(Model &mdl);
			void InitializeModelHitboxBvhCache(Model &mdl, const HitboxMeshBvhBuildTask &buildTask, ModelHitboxBvhCache &mdlHbBvhCache);
			std::unordered_map<ModelName, std::shared_ptr<ModelHitboxBvhCache>> m_modelBvhCache;
			Game &m_game;
			pragma::bvh::HitboxMeshBvhBuilder m_builder;
		};
	};
	struct HitInfo;
	struct IntersectionInfo;
	class DLLCLIENT CHitboxBvhComponent final : public BaseEntityComponent {
	  public:
		CHitboxBvhComponent(BaseEntity &ent);
		virtual ~CHitboxBvhComponent() override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		void InitializeBvh();
		bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo, const bvh::DebugDrawInfo *debugDrawInfo = nullptr) const;
		bool IntersectionTestAabb(const Vector3 &min, const Vector3 &max, IntersectionInfo *outIntersectionInfo = nullptr) const;
		bool IntersectionTestKDop(const std::vector<umath::Plane> &planes, IntersectionInfo *outIntersectionInfo = nullptr) const;
		void DebugDrawHitboxMeshes(pragma::animation::BoneId boneId, float duration = 12.f) const;
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
		std::unordered_map<pragma::animation::BoneId, std::vector<std::shared_ptr<bvh::MeshHitboxBvhCache>>> m_hitboxMeshBvhCaches;
		std::shared_ptr<bvh::ObbBvhTree> m_hitboxBvh;

		std::vector<umath::ScaledTransform> m_hitboxBvhUpdatePoses;
		std::future<void> m_hitboxBvhUpdate;
	};

	namespace bvh {
		struct DLLCLIENT HitboxObb {
			HitboxObb(const Vector3 &min, const Vector3 &max);
			pragma::bvh::BBox ToBvhBBox(const umath::ScaledTransform &pose, Vector3 &outOrigin) const;
			umath::ScaledTransform GetPose(const std::vector<umath::ScaledTransform> &effectivePoses) const;
			Vector3 position; // Position relative to bone
			Vector3 halfExtents;
			Vector3 min;
			Vector3 max;

			pragma::animation::BoneId boneId = std::numeric_limits<pragma::animation::BoneId>::max();
		};

		struct DLLCLIENT ObbBvhTree : public pragma::bvh::BvhTree {
			struct DLLCLIENT HitData {
				size_t primitiveIndex;
				float t;
			};

			void InitializeBvh(const std::vector<umath::ScaledTransform> &poses);
			bool Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, const std::vector<umath::ScaledTransform> &bonePoses, std::vector<HitData> &outHits, const bvh::DebugDrawInfo *debugDrawInfo = nullptr);
			std::vector<HitboxObb> primitives;
		  private:
			const std::vector<umath::ScaledTransform> *m_poses = nullptr;
			virtual bool DoInitializeBvh(pragma::bvh::Executor &executor, ::bvh::v2::DefaultBuilder<pragma::bvh::Node>::Config &config) override;
		};

		DLLCLIENT bool test_bvh_intersection(const ObbBvhTree &bvhData, const std::function<bool(const Vector3 &, const Vector3 &)> &testAabb, const std::function<bool(const HitboxObb &)> &testObb, size_t nodeIdx = 0, IntersectionInfo *outIntersectionInfo = nullptr);
		DLLCLIENT bool test_bvh_intersection_with_aabb(const ObbBvhTree &bvhData, const std::vector<umath::ScaledTransform> &effectivePoses, const Vector3 &min, const Vector3 &max, size_t nodeIdx = 0, IntersectionInfo *outIntersectionInfo = nullptr);
		DLLCLIENT bool test_bvh_intersection_with_obb(const ObbBvhTree &bvhData, const std::vector<umath::ScaledTransform> &effectivePoses, const Vector3 &origin, const Quat &rot, const Vector3 &min, const Vector3 &max, size_t nodeIdx = 0, IntersectionInfo *outIntersectionInfo = nullptr);
		DLLCLIENT bool test_bvh_intersection_with_kdop(const ObbBvhTree &bvhData, const std::vector<umath::ScaledTransform> &effectivePoses, const std::vector<umath::Plane> &kdop, size_t nodeIdx = 0, IntersectionInfo *outIntersectionInfo = nullptr);

	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::bvh::DebugDrawInfo::Flags)

#endif
