// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/lua/luafunction.h"
#include "pragma/physics/collisionmasks.h"

export module pragma.shared:physics.raycallback.filter;

export {
	DLLNETWORK CollisionMask get_collision_group(CollisionMask group, CollisionMask mask);

	class DLLNETWORK BasePhysRayCallbackFilter {
	protected:
		FTRACE m_flags;
		CollisionMask m_filterMask;
		CollisionMask m_filterGroup;
		mutable void *m_userData = nullptr;
		BasePhysRayCallbackFilter(FTRACE flags, CollisionMask group, CollisionMask mask);
		bool TranslateFilterValue(bool b) const;
	public:
		virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj);
		void SetUserData(void *userData) const;
		void *GetUserData() const;
	};
};
