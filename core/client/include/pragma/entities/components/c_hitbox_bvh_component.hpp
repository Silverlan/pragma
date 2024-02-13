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

		static void generate_hitbox_meshes(Model &mdl);

		CHitboxBvhComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		void InitializeBvh();
		void UpdateTest();
		bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, pragma::bvh::HitInfo &outHitInfo, const bvh::DebugDrawInfo *debugDrawInfo = nullptr);
		void UpdateHitboxBvh();
		void DebugDrawHitboxMeshes(BoneId boneId, float duration = 12.f) const;
	  private:
		struct HitboxBvhInfo {
			std::shared_ptr<pragma::bvh::MeshBvhTree> bvhTree;
			std::shared_ptr<ModelSubMesh> mesh;
			std::vector<uint32_t> bvhTriToOriginalTri;
		};
		void DebugDraw();
		void OnModelChanged();
		void InitializeHitboxBvh();
		bool InitializeModel();
		void InitializeHitboxMeshes();
		std::unordered_map<BoneId, std::vector<HitboxBvhInfo>> m_hitboxBvhs;
		std::shared_ptr<ObbBvhTree> m_hitboxBvh;
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
		virtual bool DoInitializeBvh(::bvh::v2::ParallelExecutor &executor, ::bvh::v2::DefaultBuilder<pragma::bvh::Node>::Config &config) override;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::bvh::DebugDrawInfo::Flags)

#endif
