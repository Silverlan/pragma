#include "stdafx_server.h"
#include "pragma/lua/libraries/s_lgame.h"
#include <pragma/serverstate/serverstate.h>
#include "luasystem.h"
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/libraries/ltimer.h>
#include <pragma/lua/libraries/lgame.h>
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <sharedutils/scope_guard.h>
extern DLLSERVER ServerState *server;
extern DLLSERVER SGame *s_game;
#ifdef PHYS_ENGINE_PHYSX
DLLSERVER int Lua_sv_game_GetPhysXScene(lua_State *l)
{
	Game *game = server->GetGameState();
	Lua::Push<PhysXScene>(l,PhysXScene(game->GetPhysXScene()));
	return 1;
}
#endif

DLLSERVER int Lua::game::Server::set_gravity(lua_State *l)
{
	Vector3 *gravity = Lua::CheckVector(l,1);
	Game *game = server->GetGameState();
	game->SetGravity(*gravity);
	return 0;
}
DLLSERVER int Lua::game::Server::get_gravity(lua_State *l)
{
	Game *game = server->GetGameState();
	Lua::Push<Vector3>(l,game->GetGravity());
	return 1;
}
int Lua::game::Server::load_model(lua_State *l)
{
	auto *name = Lua::CheckString(l,1);
	auto mdl = s_game->LoadModel(name);
	if(mdl == nullptr)
		return 0;
	Lua::Push<decltype(mdl)>(l,mdl);
	return 1;
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
	ScopeGuard sg([hCb]() {
		if(hCb->IsValid() == true)
			hCb->Remove();
	});

	std::string mapName;
	BaseEntity *entWorld = nullptr;
	Vector3 origin {};
	auto startIdx = s_game->GetEntityMapIndexStart();
	auto pair = Lua::game::load_map(l,mapName,&entWorld,origin);
	if(pair.first == false)
		return pair.second;
	NetPacket packet {};
	packet->WriteString(mapName);
	packet->Write<Vector3>(origin);
	packet->Write<uint32_t>(startIdx);

	nwm::RecipientFilter rp {};
	rp.SetFilterType(nwm::RecipientFilter::Type::Exclude);
	std::vector<SBaseEntity*> ptrEnts;
	ptrEnts.reserve(ents.size());
	for(auto &hEnt : ents)
	{
		if(hEnt.IsValid() == false)
			continue;
		ptrEnts.push_back(static_cast<SBaseEntity*>(hEnt.get()));
	}

	s_game->WriteEntityData(packet,ptrEnts.data(),ptrEnts.size(),rp);
	packet->Write<bool>((entWorld != nullptr) ? true : false);

	server->BroadcastTCP("map_load",packet);
	return pair.second;
}

int Lua::game::Server::change_level(lua_State *l)
{
	std::string mapName = Lua::CheckString(l,1);
	std::string landmarkName {};
	if(Lua::IsSet(l,2))
		landmarkName = Lua::CheckString(l,2);
	s_game->ChangeLevel(mapName,landmarkName);
	return 0;
}
