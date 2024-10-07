/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_bsp_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

void CBSPComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CBSPComponent::Initialize() { BaseEntityComponent::Initialize(); }
void CBSPComponent::InitializeBSPTree(source_engine::bsp::File &bsp)
{
	// TODO
	//auto bspTree = BSPTree::Create(bsp);
}
