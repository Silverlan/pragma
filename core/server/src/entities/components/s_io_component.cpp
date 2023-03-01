/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_io_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

void SIOComponent::Initialize() { BaseIOComponent::Initialize(); }
void SIOComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
