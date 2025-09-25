// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/physics/raycallback/physraycallbackfilter.hpp"
#include <functional>

export module pragma.shared:physics.raycallback.filter_function;

export class DLLNETWORK BasePhysRayCallbackFilterFunction : public BasePhysRayCallbackFilter {
  private:
	BasePhysRayCallbackFilterFunction &operator=(const BasePhysRayCallbackFilterFunction &) { return *this; };
  protected:
	std::function<bool(BaseEntity *, PhysObj *, pragma::physics::ICollisionObject *)> m_filter;
  public:
	BasePhysRayCallbackFilterFunction(const std::function<bool(BaseEntity *, PhysObj *, pragma::physics::ICollisionObject *)> &filter, FTRACE flags, CollisionMask group, CollisionMask mask);
	virtual bool ShouldPass(BaseEntity *ent, PhysObj *phys, pragma::physics::ICollisionObject *obj) override;
	void SetFilter(const std::function<bool(BaseEntity *, PhysObj *, pragma::physics::ICollisionObject *)> &f);
};
