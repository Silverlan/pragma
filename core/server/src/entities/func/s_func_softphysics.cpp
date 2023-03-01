/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/func/s_func_softphysics.hpp"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <sharedutils/netpacket.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_physics_softbody, FuncSoftPhysics);

void SFuncSoftPhysicsComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<PhysSoftBodyInfo>(m_softBodyInfo); }

void SFuncSoftPhysicsComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////

extern DLLSERVER SGame *s_game;

void FuncSoftPhysics::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFuncSoftPhysicsComponent>();
}
