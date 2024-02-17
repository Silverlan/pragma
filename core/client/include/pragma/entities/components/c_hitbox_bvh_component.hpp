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
	class ObbBvhTree;
	namespace bvh {
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

			std::unordered_map<BoneId, std::shared_ptr<BoneHitboxBvhCache>> boneCache;
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
	class DLLCLIENT CHitboxBvhComponent final : public BaseEntityComponent {
	  public:
		struct DLLCLIENT HitboxObb {
			HitboxObb(const Vector3 &min, const Vector3 &max);
			pragma::bvh::BBox ToBvhBBox(const umath::ScaledTransform &pose, Vector3 &outOrigin) const;
			Vector3 position; // Position relative to bone
			Vector3 halfExtents;
			Vector3 min;
			Vector3 max;

			BoneId boneId = std::numeric_limits<uint16_t>::max();
		};

		CHitboxBvhComponent(BaseEntity &ent);
		virtual ~CHitboxBvhComponent() override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		void InitializeBvh();
		bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, pragma::bvh::HitInfo &outHitInfo, const bvh::DebugDrawInfo *debugDrawInfo = nullptr);
		void DebugDrawHitboxMeshes(BoneId boneId, float duration = 12.f) const;
		bvh::HitboxBvhCache &GetGlobalBvhCache() const;
	  private:
		void Reset();
		void WaitForHitboxBvhUpdate();
		void DebugDraw();
		void OnModelChanged();
		void InitializeHitboxBvh();
		bool InitializeModel();
		void InitializeHitboxMeshBvhs();
		void UpdateHitboxBvh();
		std::shared_future<void> m_hitboxMeshCacheTask;
		std::unordered_map<BoneId, std::vector<std::shared_ptr<bvh::MeshHitboxBvhCache>>> m_hitboxMeshBvhCaches;
		std::shared_ptr<ObbBvhTree> m_hitboxBvh;

		std::vector<umath::ScaledTransform> m_hitboxBvhUpdatePoses;
		std::future<void> m_hitboxBvhUpdate;
	};

	struct DLLCLIENT ObbBvhTree : public pragma::bvh::BvhTree {
		struct DLLCLIENT HitData {
			size_t primitiveIndex;
			float t;
		};

		void InitializeBvh(const std::vector<umath::ScaledTransform> &poses);
		bool Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, const std::vector<umath::ScaledTransform> &bonePoses, std::vector<HitData> &outHits, const bvh::DebugDrawInfo *debugDrawInfo = nullptr);
		std::vector<pragma::CHitboxBvhComponent::HitboxObb> primitives;
	  private:
		const std::vector<umath::ScaledTransform> *m_poses = nullptr;
		virtual bool DoInitializeBvh(pragma::bvh::Executor &executor, ::bvh::v2::DefaultBuilder<pragma::bvh::Node>::Config &config) override;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::bvh::DebugDrawInfo::Flags)

#endif
