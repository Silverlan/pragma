// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.client;


import :entities.components.bsp;
import :engine;


using namespace pragma;

void CBSPComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CBSPComponent::Initialize() { BaseEntityComponent::Initialize(); }
void CBSPComponent::InitializeBSPTree(source_engine::bsp::File &bsp)
{
	// TODO
	//auto bspTree = BSPTree::Create(bsp);
}
