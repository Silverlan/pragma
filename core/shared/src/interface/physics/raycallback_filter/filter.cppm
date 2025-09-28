// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:physics.raycallback.filter;

export import :physics.enums;
export import :physics.collision_object;
export import :physics.object;

export {
	class BaseEntity;
	DLLNETWORK CollisionMask get_collision_group(CollisionMask group, CollisionMask mask);

	class DLLNETWORK BasePhysRayCallbackFilter {
	protected:
		RayCastFlags m_flags;
		CollisionMask m_filterMask;
		CollisionMask m_filterGroup;
		mutable void *m_userData = nullptr;
		BasePhysRayCallbackFilter(RayCastFlags flags, CollisionMask group, CollisionMask mask);
		bool TranslateFilterValue(bool b) const;
	public:
		virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj);
		void SetUserData(void *userData) const;
		void *GetUserData() const;
	};
};
