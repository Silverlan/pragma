// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/info/s_info_landmark.hpp"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(info_landmark, InfoLandmark);

void SInfoLandmarkComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void InfoLandmark::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SInfoLandmarkComponent>();
}
