/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/lua/libraries/s_lgame.h"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/serverstate/serverstate.h>
#include "luasystem.h"
#include <pragma/networking/enums.hpp>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/libraries/ltimer.h>
#include <pragma/lua/libraries/lgame.h>
#include <pragma/model/model.h>
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <sharedutils/scope_guard.h>
extern DLLSERVER ServerState *server;
extern DLLSERVER SGame *s_game;

void Lua::game::Server::set_gravity(const Vector3 &gravity)
{
	Game *game = server->GetGameState();
	game->SetGravity(const_cast<Vector3&>(gravity));
}
Vector3 Lua::game::Server::get_gravity()
{
	Game *game = server->GetGameState();
	return game->GetGravity();
}
LuaModelObject Lua::game::Server::load_model(lua_State *l,const std::string &name)
{
	auto mdl = s_game->LoadModel(name);
	if(mdl == nullptr)
		return {};
	return luabind::object{l,mdl};
}
int Lua::game::Server::create_model(lua_State *l)
{
	std::shared_ptr<Model> mdl = nullptr;
	if(!Lua::IsSet(l,1))
		mdl = s_game->CreateModel();
	else
	{
		if(Lua::IsBool(l,1))
		{
			auto bAddReference = true;
			if(Lua::IsSet(l,1))
				bAddReference = Lua::CheckBool(l,1);
			mdl = s_game->CreateModel(bAddReference);
		}
		else
		{
			std::string name = Lua::CheckString(l,1);
			mdl = s_game->CreateModel(name);
		}
	}
	if(mdl == nullptr)
		return 0;
	Lua::Push<decltype(mdl)>(l,mdl);
	return 1;
}

int Lua::game::Server::load_map(lua_State *l)
{
	std::vector<EntityHandle> ents;
	auto hCb = std::make_shared<CallbackHandle>(s_game->AddCallback("OnEntityCreated",FunctionCallback<void,BaseEntity*>::Create([&ents](BaseEntity *ent) {
		ents.push_back(ent->GetHandle());
	})));
	util::ScopeGuard sg([hCb]() {
		if(hCb->IsValid() == true)
			hCb->Remove();
	});

	std::string mapName;
	BaseEntity *entWorld = nullptr;
	Vector3 origin {};
	//auto startIdx = s_game->GetEntityMapIndexStart();
	auto pair = Lua::game::load_map(l,mapName,&entWorld,origin);
	if(pair.first == false)
		return pair.second;
	NetPacket packet {};
	packet->WriteString(mapName);
	packet->Write<Vector3>(origin);
	//packet->Write<uint32_t>(startIdx);

	std::vector<SBaseEntity*> ptrEnts;
	ptrEnts.reserve(ents.size());
	for(auto &hEnt : ents)
	{
		if(hEnt.valid() == false)
			continue;
		ptrEnts.push_back(static_cast<SBaseEntity*>(hEnt.get()));
	}

	pragma::networking::ClientRecipientFilter filter{};
	s_game->WriteEntityData(packet,ptrEnts.data(),ptrEnts.size(),filter);
	packet->Write<bool>((entWorld != nullptr) ? true : false);

	server->SendPacket("map_load",packet,pragma::networking::Protocol::SlowReliable);
	return pair.second;
}

void Lua::game::Server::change_level(const std::string &mapName,const std::string &landmarkName) {s_game->ChangeLevel(mapName,landmarkName);}
void Lua::game::Server::change_level(const std::string &mapName) {change_level(mapName,"");}
