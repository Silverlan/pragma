// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components.point_target;

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_target, CPointTarget);

void CPointTargetComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CPointTarget::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CTransformComponent>();
	AddComponent<CPointTargetComponent>();
}
