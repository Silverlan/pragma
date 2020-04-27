/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_health_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

luabind::object CHealthComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CHealthComponentHandleWrapper>(l);}
void CHealthComponent::ReceiveData(NetPacket &packet)
{
	*m_health = packet->Read<unsigned short>();
}
