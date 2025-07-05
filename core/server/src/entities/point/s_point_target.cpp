// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/point/s_point_target.h"
#include "pragma/entities/components/s_transform_component.hpp"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_target, PointTarget);

void SPointTargetComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointTarget::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STransformComponent>();
	AddComponent<SPointTargetComponent>();
}
