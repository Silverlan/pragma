// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.raycast_filter;

pragma::physics::EntityRayCastFilterCallback::EntityRayCastFilterCallback(ecs::BaseEntity &ent) : m_hEnt {ent.GetHandle()} {}
pragma::physics::RayCastHitType pragma::physics::EntityRayCastFilterCallback::PreFilter(IShape &shape, IRigidBody &rigidBody) const
{
	auto *physObj = rigidBody.GetPhysObj();
	auto *ent = physObj ? physObj->GetOwner() : nullptr;
	return (ent && m_hEnt.get() == &ent->GetEntity()) ? RayCastHitType::Block : RayCastHitType::None;
}
pragma::physics::RayCastHitType pragma::physics::EntityRayCastFilterCallback::PostFilter(IShape &shape, IRigidBody &rigidBody) const { return RayCastHitType::None; }
bool pragma::physics::EntityRayCastFilterCallback::HasPreFilter() const { return true; }
bool pragma::physics::EntityRayCastFilterCallback::HasPostFilter() const { return false; }

//////////////////

pragma::physics::MultiEntityRayCastFilterCallback::MultiEntityRayCastFilterCallback(std::vector<EntityHandle> &&ents) : m_ents {std::move(ents)} {}
pragma::physics::RayCastHitType pragma::physics::MultiEntityRayCastFilterCallback::PreFilter(IShape &shape, IRigidBody &rigidBody) const
{
	auto *physObj = rigidBody.GetPhysObj();
	auto *ent = physObj ? physObj->GetOwner() : nullptr;
	return (ent && std::find_if(m_ents.begin(), m_ents.end(), [ent](const EntityHandle &hEntOther) { return hEntOther.get() == &ent->GetEntity(); }) != m_ents.end()) ? RayCastHitType::Block : RayCastHitType::None;
}
pragma::physics::RayCastHitType pragma::physics::MultiEntityRayCastFilterCallback::PostFilter(IShape &shape, IRigidBody &rigidBody) const { return RayCastHitType::None; }
bool pragma::physics::MultiEntityRayCastFilterCallback::HasPreFilter() const { return true; }
bool pragma::physics::MultiEntityRayCastFilterCallback::HasPostFilter() const { return false; }

//////////////////

pragma::physics::PhysObjRayCastFilterCallback::PhysObjRayCastFilterCallback(PhysObj &physObj) : m_hPhys {physObj.GetHandle()} {}
pragma::physics::RayCastHitType pragma::physics::PhysObjRayCastFilterCallback::PreFilter(IShape &shape, IRigidBody &rigidBody) const
{
	auto *physObj = rigidBody.GetPhysObj();
	return (physObj && m_hPhys.get() == physObj) ? RayCastHitType::Block : RayCastHitType::None;
}
pragma::physics::RayCastHitType pragma::physics::PhysObjRayCastFilterCallback::PostFilter(IShape &shape, IRigidBody &rigidBody) const { return RayCastHitType::None; }
bool pragma::physics::PhysObjRayCastFilterCallback::HasPreFilter() const { return true; }
bool pragma::physics::PhysObjRayCastFilterCallback::HasPostFilter() const { return false; }

//////////////////

pragma::physics::CollisionObjRayCastFilterCallback::CollisionObjRayCastFilterCallback(ICollisionObject &colObj) : m_hColObj {util::weak_shared_handle_cast<IBase, ICollisionObject>(colObj.GetHandle())} {}
pragma::physics::RayCastHitType pragma::physics::CollisionObjRayCastFilterCallback::PreFilter(IShape &shape, IRigidBody &rigidBody) const
{
	return (&rigidBody == m_hColObj.Get()) ? RayCastHitType::Block : RayCastHitType::None;
}
pragma::physics::RayCastHitType pragma::physics::CollisionObjRayCastFilterCallback::PostFilter(IShape &shape, IRigidBody &rigidBody) const { return RayCastHitType::None; }
bool pragma::physics::CollisionObjRayCastFilterCallback::HasPreFilter() const { return true; }
bool pragma::physics::CollisionObjRayCastFilterCallback::HasPostFilter() const { return false; }

//////////////////

pragma::physics::CustomRayCastFilterCallback::CustomRayCastFilterCallback(const std::function<RayCastHitType(IShape &, IRigidBody &)> &preFilter,
  const std::function<RayCastHitType(IShape &, IRigidBody &)> &postFilter)
    : m_preFilter {preFilter}, m_postFilter {postFilter}
{
}
pragma::physics::RayCastHitType pragma::physics::CustomRayCastFilterCallback::PreFilter(IShape &shape, IRigidBody &rigidBody) const { return m_preFilter(shape, rigidBody); }
pragma::physics::RayCastHitType pragma::physics::CustomRayCastFilterCallback::PostFilter(IShape &shape, IRigidBody &rigidBody) const { return m_postFilter(shape, rigidBody); }
bool pragma::physics::CustomRayCastFilterCallback::HasPreFilter() const { return m_preFilter != nullptr; }
bool pragma::physics::CustomRayCastFilterCallback::HasPostFilter() const { return m_postFilter != nullptr; }
