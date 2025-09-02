// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.server.entities.components.io;

using namespace pragma;

void SIOComponent::Initialize() { BaseIOComponent::Initialize(); }
void SIOComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
