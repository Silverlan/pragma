// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/environment/s_env_camera.h"
#include "pragma/entities/components/s_field_angle_component.hpp"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_camera, EnvCamera);

void SCameraComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SCameraComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvCameraComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(SFieldAngleComponent))
		SetFieldAngleComponent(static_cast<SFieldAngleComponent &>(component));
}

void EnvCamera::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SCameraComponent>();
}
