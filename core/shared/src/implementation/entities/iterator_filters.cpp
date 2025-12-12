// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.iterator;

EntityIteratorFilterName::EntityIteratorFilterName(pragma::Game &game, const std::string &name, bool caseSensitive, bool exactMatch) : m_name(name), m_bCaseSensitive(caseSensitive), m_bExactMatch(exactMatch) {}
bool EntityIteratorFilterName::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index)
{
	auto pNameComponent = static_cast<pragma::BaseNameComponent *>(ent.FindComponent("name").get());
	if(pNameComponent == nullptr)
		return false;
	return m_bExactMatch ? pragma::string::match(pNameComponent->GetName(), m_name, m_bCaseSensitive) : pragma::string::compare(pNameComponent->GetName(), m_name, m_bCaseSensitive);
}

/////////////////

EntityIteratorFilterModel::EntityIteratorFilterModel(pragma::Game &game, const std::string &mdlName) : m_modelName {mdlName} {}
bool EntityIteratorFilterModel::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index)
{
	auto pMdlComponent = static_cast<pragma::BaseModelComponent *>(ent.FindComponent("model").get());
	if(pMdlComponent == nullptr)
		return false;
	return pragma::asset::matches(m_modelName, pMdlComponent->GetModelName(), pragma::asset::Type::Model);
}

/////////////////

EntityIteratorFilterUuid::EntityIteratorFilterUuid(pragma::Game &game, const pragma::util::Uuid &uuid) : m_uuid {uuid} {}
bool EntityIteratorFilterUuid::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) { return ent.GetUuid() == m_uuid; }

/////////////////

EntityIteratorFilterClass::EntityIteratorFilterClass(pragma::Game &game, const std::string &name, bool caseSensitive, bool exactMatch) : m_name(name), m_bCaseSensitive(caseSensitive), m_bExactMatch(exactMatch) {}
bool EntityIteratorFilterClass::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) { return m_bExactMatch ? pragma::string::match(*ent.GetClass(), m_name.c_str(), m_bCaseSensitive) : pragma::string::compare(ent.GetClass().c_str(), m_name.c_str(), m_bCaseSensitive); }

/////////////////

EntityIteratorFilterNameOrClass::EntityIteratorFilterNameOrClass(pragma::Game &game, const std::string &name, bool caseSensitive, bool exactMatch) : m_name(name), m_bCaseSensitive(caseSensitive), m_bExactMatch(exactMatch) {}
bool EntityIteratorFilterNameOrClass::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index)
{
	if(m_bExactMatch ? pragma::string::match(*ent.GetClass(), m_name, m_bCaseSensitive) : pragma::string::compare(*ent.GetClass(), m_name.c_str(), m_bCaseSensitive))
		return true;
	auto pNameComponent = static_cast<pragma::BaseNameComponent *>(ent.FindComponent("name").get());
	return pNameComponent != nullptr && (m_bExactMatch ? pragma::string::match(pNameComponent->GetName(), m_name, m_bCaseSensitive) : pragma::string::compare(pNameComponent->GetName(), m_name, m_bCaseSensitive));
}

/////////////////

EntityIteratorFilterEntity::EntityIteratorFilterEntity(pragma::Game &game, const std::string &name) : m_name(name)
{
	auto &componentManager = game.GetEntityComponentManager();
	componentManager.GetComponentTypeId("filter_name", m_filterNameComponentId);
	componentManager.GetComponentTypeId("filter_class", m_filterClassComponentId);

	pragma::ecs::EntityIterator entIt {game};
	entIt.AttachFilter<EntityIteratorFilterName>(name);
	for(auto *ent : entIt) {
		auto pFilterComponent = dynamic_cast<pragma::BaseFilterComponent *>(ent->FindComponent(m_filterNameComponentId).get());
		if(pFilterComponent == nullptr)
			pFilterComponent = dynamic_cast<pragma::BaseFilterComponent *>(ent->FindComponent(m_filterClassComponentId).get());
		if(pFilterComponent == nullptr)
			continue;
		m_filterEnts.push_back(std::static_pointer_cast<pragma::BaseFilterComponent>(pFilterComponent->shared_from_this()));
	}
}
bool EntityIteratorFilterEntity::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index)
{
	for(auto &hFilter : m_filterEnts) {
		if(hFilter.expired())
			continue;
		if(hFilter->ShouldPass(ent))
			return true;
	}
	if(pragma::string::compare(ent.GetClass().c_str(), m_name.c_str(), false))
		return true;
	auto pNameComponent = static_cast<pragma::BaseNameComponent *>(ent.FindComponent("name").get());
	return pNameComponent != nullptr && pragma::string::compare(pNameComponent->GetName(), m_name, false);
}

/////////////////

EntityIteratorFilterFlags::EntityIteratorFilterFlags(pragma::Game &game, pragma::ecs::EntityIterator::FilterFlags flags) : m_flags(flags) {}
bool EntityIteratorFilterFlags::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index)
{
	auto bIncludeEntity = false;
	if(ent.IsSpawned()) {
		if((m_flags & pragma::ecs::EntityIterator::FilterFlags::Spawned) != pragma::ecs::EntityIterator::FilterFlags::None)
			bIncludeEntity = true;
	}
	else if((m_flags & pragma::ecs::EntityIterator::FilterFlags::Pending) != pragma::ecs::EntityIterator::FilterFlags::None)
		bIncludeEntity = true;
	if(bIncludeEntity == false)
		return false;

	if(ent.IsNetworkLocal() == false) {
		if((m_flags & pragma::ecs::EntityIterator::FilterFlags::IncludeShared) != pragma::ecs::EntityIterator::FilterFlags::None)
			bIncludeEntity = true;
	}
	else if((m_flags & pragma::ecs::EntityIterator::FilterFlags::IncludeNetworkLocal) != pragma::ecs::EntityIterator::FilterFlags::None)
		bIncludeEntity = true;
	if(bIncludeEntity == false || (m_flags & pragma::ecs::EntityIterator::FilterFlags::AnyType) == pragma::ecs::EntityIterator::FilterFlags::AnyType || (m_flags & pragma::ecs::EntityIterator::FilterFlags::AnyType) == pragma::ecs::EntityIterator::FilterFlags::None) {
		if((m_flags & pragma::ecs::EntityIterator::FilterFlags::HasTransform) != pragma::ecs::EntityIterator::FilterFlags::None && ent.GetTransformComponent())
			return true;

		if((m_flags & pragma::ecs::EntityIterator::FilterFlags::HasModel) != pragma::ecs::EntityIterator::FilterFlags::None && ent.GetModelComponent())
			return true;
		return bIncludeEntity;
	}
	if((m_flags & pragma::ecs::EntityIterator::FilterFlags::HasTransform) != pragma::ecs::EntityIterator::FilterFlags::None && !ent.GetTransformComponent())
		return false;
	if((m_flags & pragma::ecs::EntityIterator::FilterFlags::HasModel) != pragma::ecs::EntityIterator::FilterFlags::None && !ent.GetModelComponent())
		return false;

	if((m_flags & pragma::ecs::EntityIterator::FilterFlags::Character) != pragma::ecs::EntityIterator::FilterFlags::None && ent.IsCharacter())
		return true;
	if((m_flags & pragma::ecs::EntityIterator::FilterFlags::Player) != pragma::ecs::EntityIterator::FilterFlags::None && ent.IsPlayer())
		return true;
	if((m_flags & pragma::ecs::EntityIterator::FilterFlags::Weapon) != pragma::ecs::EntityIterator::FilterFlags::None && ent.IsWeapon())
		return true;
	if((m_flags & pragma::ecs::EntityIterator::FilterFlags::Vehicle) != pragma::ecs::EntityIterator::FilterFlags::None && ent.IsVehicle())
		return true;
	if((m_flags & pragma::ecs::EntityIterator::FilterFlags::NPC) != pragma::ecs::EntityIterator::FilterFlags::None && ent.IsNPC())
		return true;
	if((m_flags & pragma::ecs::EntityIterator::FilterFlags::Scripted) != pragma::ecs::EntityIterator::FilterFlags::None && ent.IsScripted())
		return true;
	if((m_flags & pragma::ecs::EntityIterator::FilterFlags::MapEntity) != pragma::ecs::EntityIterator::FilterFlags::None && ent.IsMapEntity())
		return true;
	if((m_flags & pragma::ecs::EntityIterator::FilterFlags::Physical) != pragma::ecs::EntityIterator::FilterFlags::None) {
		auto pPhysComponent = ent.GetPhysicsComponent();
		if(pPhysComponent != nullptr && pPhysComponent->GetPhysicsObject() != nullptr)
			return true;
	}
	return false;
}

/////////////////

EntityIteratorFilterComponent::EntityIteratorFilterComponent(pragma::Game &game, pragma::ComponentId componentId) : m_componentId(componentId) {}
EntityIteratorFilterComponent::EntityIteratorFilterComponent(pragma::Game &game, const std::string &componentName)
{
	auto &componentManager = game.GetEntityComponentManager();
	componentManager.GetComponentTypeId(componentName, m_componentId);
}

bool EntityIteratorFilterComponent::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) { return ent.HasComponent(m_componentId); }

/////////////////

EntityIteratorFilterUser::EntityIteratorFilterUser(pragma::Game &game, const std::function<bool(pragma::ecs::BaseEntity &, std::size_t)> &fUserFilter) : m_fUserFilter(fUserFilter) {}

bool EntityIteratorFilterUser::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) { return m_fUserFilter(ent, index); }

/////////////////

EntityIteratorFilterSphere::EntityIteratorFilterSphere(pragma::Game &game, const Vector3 &origin, float radius) : m_origin(origin), m_radius(radius) {}

bool EntityIteratorFilterSphere::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index, Vector3 &outClosestPointOnEntityBounds, float &outDistToEntity) const
{
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent == nullptr)
		return false;
	auto pPhysComponent = ent.GetPhysicsComponent();
	Vector3 colCenter;
	auto &pos = pTrComponent->GetPosition();
	auto colRadius = pPhysComponent != nullptr ? pPhysComponent->GetCollisionRadius(&colCenter) : 0.f;
	colCenter += pos;
	auto distToCol = uvec::distance(m_origin, colCenter) - colRadius;
	if(distToCol > m_radius)
		return false;
	Vector3 min {};
	Vector3 max {};
	if(pPhysComponent != nullptr)
		pPhysComponent->GetCollisionBounds(&min, &max);
	pragma::math::geometry::closest_point_on_aabb_to_point(min, max, m_origin - pos, &outClosestPointOnEntityBounds);
	outDistToEntity = uvec::length(outClosestPointOnEntityBounds);
	return outDistToEntity <= m_radius;
}

bool EntityIteratorFilterSphere::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index)
{
	Vector3 r;
	float d;
	return ShouldPass(ent, index, r, d);
}

/////////////////

EntityIteratorFilterBox::EntityIteratorFilterBox(pragma::Game &game, const Vector3 &min, const Vector3 &max) : m_min(min), m_max(max) {}

bool EntityIteratorFilterBox::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index)
{
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent == nullptr)
		return false;
	auto pPhysComponent = ent.GetPhysicsComponent();
	Vector3 entMin {};
	Vector3 entMax {};
	if(pPhysComponent != nullptr)
		pPhysComponent->GetCollisionBounds(&entMin, &entMax);
	return pragma::math::intersection::aabb_aabb(m_min, m_max, entMin, entMax) != pragma::math::intersection::Intersect::Outside;
}

/////////////////

EntityIteratorFilterCone::EntityIteratorFilterCone(pragma::Game &game, const Vector3 &origin, const Vector3 &dir, float radius, float angle)
    : EntityIteratorFilterSphere(game, origin, radius), m_direction(dir), m_angle(static_cast<float>(pragma::math::cos(static_cast<float>(pragma::math::deg_to_rad(angle)))))
{
}

bool EntityIteratorFilterCone::ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index)
{
	Vector3 pos;
	float dist;
	if(EntityIteratorFilterSphere::ShouldPass(ent, index, pos, dist) == false)
		return false;
	auto dirToOrigin = pos - m_origin;
	uvec::normalize(&dirToOrigin);
	auto dot = uvec::dot(m_direction, dirToOrigin);
	return dot < m_angle;
}
