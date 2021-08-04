/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/lua/classes/s_lua_entity.h"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/networking/iserver_client.hpp"
#include <pragma/entities/components/map_component.hpp>
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/networking/nwm_util.h>

extern DLLSERVER SGame *s_game;
extern DLLSERVER ServerState *server;
#pragma optimize("",off)
SLuaEntity::SLuaEntity(luabind::object &o,const std::string &className)
	: SBaseEntity{},LuaObjectBase{}
{
	m_class = className;
	SBaseEntity::SetLuaObject(o);
	LuaObjectBase::SetLuaObject(o);
}
void SLuaEntity::Initialize()
{
	SBaseEntity::Initialize();
	CallLuaMember("Initialize");
}
bool SLuaEntity::IsScripted() const {return true;}
void SLuaEntity::InitializeLuaObject(lua_State *lua)
{
	pragma::BaseLuaHandle::InitializeLuaObject<SLuaEntity>(lua);
}

void SLuaEntity::DoSpawn()
{
	SBaseEntity::DoSpawn();
	if(IsShared())
	{
		pragma::networking::ClientRecipientFilter rf {[](const pragma::networking::IServerClient &cl) -> bool {
			return cl.GetPlayer();
		}};
		NetPacket p;
		p->WriteString(GetClass());
		p->Write<unsigned int>(GetIndex());
		auto pMapComponent = GetComponent<pragma::MapComponent>();
		p->Write<unsigned int>(pMapComponent.valid() ? pMapComponent->GetMapIndex() : 0u);
		SendData(p,rf);
		server->SendPacket("ent_create_lua",p,pragma::networking::Protocol::SlowReliable,rf);
	}
}
void SLuaEntity::Remove()
{
	if(IsSpawned())
	{
		// TODO: Do we need this? (If so, why?)
		NetPacket p;
		nwm::write_entity(p,this);
		server->SendPacket("ent_remove",p,pragma::networking::Protocol::SlowReliable);
	}
	SBaseEntity::Remove();
}
void SLuaEntity::default_Initialize(SBaseEntity *ent) {}
#pragma optimize("",on)
