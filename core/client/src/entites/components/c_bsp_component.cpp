/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_bsp_component.hpp"

extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

luabind::object CBSPComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CBSPComponentHandleWrapper>(l);}
void CBSPComponent::Initialize()
{
	BaseEntityComponent::Initialize();

}
void CBSPComponent::InitializeBSPTree(bsp::File &bsp)
{
	// TODO
	//auto bspTree = BSPTree::Create(bsp);

}
