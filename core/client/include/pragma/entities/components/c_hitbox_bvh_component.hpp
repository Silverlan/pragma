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
	class DLLCLIENT CHitboxBvhComponent final : public BaseEntityComponent {
	  public:
		static void generate_hitbox_meshes(Model &mdl);

		CHitboxBvhComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;
		void InitializeBvh();
	  private:
		void DebugDraw();
		void OnModelChanged();
		bool InitializeModel();
		void InitializeHitboxMeshes();
		void IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, pragma::bvh::HitInfo &outHitInfo);
		std::unordered_map<BoneId, std::shared_ptr<pragma::bvh::BvhData>> m_hitboxBvhs;
	};
};

#endif
