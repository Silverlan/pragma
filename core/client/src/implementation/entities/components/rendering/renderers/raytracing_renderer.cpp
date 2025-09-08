// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include "pragma/c_engine.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"

module pragma.client.entities.components.raytracing_renderer;

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

void CRaytracingRendererComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////

void CRaytracingRenderer::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CRaytracingRendererComponent>();
}
