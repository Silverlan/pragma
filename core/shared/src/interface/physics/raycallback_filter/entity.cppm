// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include "pragma/entities/baseentity_handle.h"

export module pragma.shared:physics.raycallback.filter_entity;

export class DLLNETWORK BasePhysRayCallbackFilterEntity : public BasePhysRayCallbackFilter {
  protected:
	std::vector<EntityHandle> m_filter;
  public:
	BasePhysRayCallbackFilterEntity(const std::vector<EntityHandle> &filter, FTRACE flags, CollisionMask group, CollisionMask mask);
	BasePhysRayCallbackFilterEntity(const EntityHandle &filter, FTRACE flags, CollisionMask group, CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj) override;
};
