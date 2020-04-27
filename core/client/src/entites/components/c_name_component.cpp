/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_name_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

void CNameComponent::ReceiveData(NetPacket &packet)
{
	std::string name = packet->ReadString();
	SetName(name);
}
luabind::object CNameComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CNameComponentHandleWrapper>(l);}
