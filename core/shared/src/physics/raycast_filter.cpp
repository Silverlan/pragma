/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/raycast_filter.hpp"
#include "pragma/physics/raytraces.h"

pragma::physics::EntityRayCastFilterCallback::EntityRayCastFilterCallback(BaseEntity &ent) : m_hEnt {ent.GetHandle()} {}
RayCastHitType pragma::physics::EntityRayCastFilterCallback::PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const
{
	auto *physObj = rigidBody.GetPhysObj();
	auto *ent = physObj ? physObj->GetOwner() : nullptr;
	return (ent && m_hEnt.get() == &ent->GetEntity()) ? RayCastHitType::Block : RayCastHitType::None;
}
RayCastHitType pragma::physics::EntityRayCastFilterCallback::PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const { return RayCastHitType::None; }
bool pragma::physics::EntityRayCastFilterCallback::HasPreFilter() const { return true; }
bool pragma::physics::EntityRayCastFilterCallback::HasPostFilter() const { return false; }

//////////////////

pragma::physics::MultiEntityRayCastFilterCallback::MultiEntityRayCastFilterCallback(std::vector<EntityHandle> &&ents) : m_ents {std::move(ents)} {}
RayCastHitType pragma::physics::MultiEntityRayCastFilterCallback::PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const
{
	auto *physObj = rigidBody.GetPhysObj();
	auto *ent = physObj ? physObj->GetOwner() : nullptr;
	return (ent && std::find_if(m_ents.begin(), m_ents.end(), [ent](const EntityHandle &hEntOther) { return hEntOther.get() == &ent->GetEntity(); }) != m_ents.end()) ? RayCastHitType::Block : RayCastHitType::None;
}
RayCastHitType pragma::physics::MultiEntityRayCastFilterCallback::PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const { return RayCastHitType::None; }
bool pragma::physics::MultiEntityRayCastFilterCallback::HasPreFilter() const { return true; }
bool pragma::physics::MultiEntityRayCastFilterCallback::HasPostFilter() const { return false; }

//////////////////

pragma::physics::PhysObjRayCastFilterCallback::PhysObjRayCastFilterCallback(PhysObj &physObj) : m_hPhys {physObj.GetHandle()} {}
RayCastHitType pragma::physics::PhysObjRayCastFilterCallback::PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const
{
	auto *physObj = rigidBody.GetPhysObj();
	return (physObj && m_hPhys.get() == physObj) ? RayCastHitType::Block : RayCastHitType::None;
}
RayCastHitType pragma::physics::PhysObjRayCastFilterCallback::PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const { return RayCastHitType::None; }
bool pragma::physics::PhysObjRayCastFilterCallback::HasPreFilter() const { return true; }
bool pragma::physics::PhysObjRayCastFilterCallback::HasPostFilter() const { return false; }

//////////////////

pragma::physics::CollisionObjRayCastFilterCallback::CollisionObjRayCastFilterCallback(ICollisionObject &colObj) : m_hColObj {util::weak_shared_handle_cast<IBase, ICollisionObject>(colObj.GetHandle())} {}
RayCastHitType pragma::physics::CollisionObjRayCastFilterCallback::PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const { return (&rigidBody == m_hColObj.Get()) ? RayCastHitType::Block : RayCastHitType::None; }
RayCastHitType pragma::physics::CollisionObjRayCastFilterCallback::PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const { return RayCastHitType::None; }
bool pragma::physics::CollisionObjRayCastFilterCallback::HasPreFilter() const { return true; }
bool pragma::physics::CollisionObjRayCastFilterCallback::HasPostFilter() const { return false; }

//////////////////

pragma::physics::CustomRayCastFilterCallback::CustomRayCastFilterCallback(const std::function<RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &preFilter, const std::function<RayCastHitType(pragma::physics::IShape &, pragma::physics::IRigidBody &)> &postFilter)
    : m_preFilter {preFilter}, m_postFilter {postFilter}
{
}
RayCastHitType pragma::physics::CustomRayCastFilterCallback::PreFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const { return m_preFilter(shape, rigidBody); }
RayCastHitType pragma::physics::CustomRayCastFilterCallback::PostFilter(pragma::physics::IShape &shape, pragma::physics::IRigidBody &rigidBody) const { return m_postFilter(shape, rigidBody); }
bool pragma::physics::CustomRayCastFilterCallback::HasPreFilter() const { return m_preFilter != nullptr; }
bool pragma::physics::CustomRayCastFilterCallback::HasPostFilter() const { return m_postFilter != nullptr; }
