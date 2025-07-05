// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/c_engine.h"
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
