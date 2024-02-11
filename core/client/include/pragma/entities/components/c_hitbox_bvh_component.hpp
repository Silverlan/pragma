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
	class DLLCLIENT CHitboxBvhComponent final : public BaseEntityComponent {
	  public:
		struct DLLCLIENT HitboxObb {
			HitboxObb(const Vector3 &min, const Vector3 &max);
			pragma::bvh::BBox ToBvhBBox(Vector3 &outOrigin) const;
			umath::ScaledTransform pose;
			Vector3 position; // Position relative to bone
			Vector3 halfExtents;
			Vector3 min;
			Vector3 max;

			BoneId boneId;
		};

		static void generate_hitbox_meshes(Model &mdl);

		CHitboxBvhComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		void InitializeBvh();
		void UpdateTest();
		void IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, pragma::bvh::HitInfo &outHitInfo);
	  private:
		void DebugDraw();
		void OnModelChanged();
		void InitializeHitboxBvh();
		void UpdateHitboxBvh();
		bool InitializeModel();
		void InitializeHitboxMeshes();
		std::unordered_map<BoneId, std::shared_ptr<pragma::bvh::MeshBvhTree>> m_hitboxBvhs;
		std::shared_ptr<ObbBvhTree> m_hitboxBvh;
	};

	struct DLLCLIENT ObbBvhTree : public pragma::bvh::BvhTree {
		struct DLLCLIENT HitData {
			size_t primitiveIndex;
			float t;
		};

		bool Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, std::vector<HitData> &outHits);
		std::vector<pragma::CHitboxBvhComponent::HitboxObb> primitives;
	  private:
		virtual bool DoInitializeBvh(::bvh::v2::ParallelExecutor &executor, ::bvh::v2::DefaultBuilder<pragma::bvh::Node>::Config &config) override;
	};
};

#endif
