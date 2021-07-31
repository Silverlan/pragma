/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include <pragma/engine.h>
#include "pragma/console/s_cvar_global.h"
#include "pragma/game/s_game.h"
#include "pragma/console/cvar_global.h"
#include "pragma/entities/player.h"
#include "pragma/networking/iserver.hpp"
#include "pragma/networking/iserver_client.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include <pragma/entities/components/base_name_component.hpp>
#include <pragma/console/util_cmd.hpp>
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/util.hpp>
#include <pragma/asset/util_asset.hpp>
#include <sharedutils/util_file.h>
#include <pragma/networking/netmessages.h>
#include <pragma/entities/entity_iterator.hpp>

extern DLLNETWORK Engine *engine;
extern DLLSERVER ServerState *server;
extern DLLSERVER SGame *s_game;
DLLSERVER void CMD_lua_run(NetworkState *state,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	if(argv.empty() || !state->IsGameActive()) return;
	Game *game = state->GetGameState();
	if(game == NULL)
		return;
	std::string lua = argv[0];
	for(auto i=1;i<argv.size();i++)
	{
		lua += " ";
		lua += argv[i];
	}
	game->RunLua(lua);
}

void CMD_drop(NetworkState*,pragma::BasePlayerComponent *pl,std::vector<std::string>&)
{
	if(pl == nullptr)
		return;
	auto sCharComponent = pl->GetEntity().GetComponent<pragma::SCharacterComponent>();
	if(sCharComponent.expired() == false)
		sCharComponent.get()->DropActiveWeapon();
}

void CMD_kick(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	if(argv.empty() || argv.front().empty())
		return;
	pragma::SPlayerComponent *kickTarget = nullptr;
	auto &identifier = argv.front();
	auto name = identifier +"*";
	EntityIterator entIt {*s_game};
	entIt.AttachFilter<EntityIteratorFilterName>(name,false,false);
	auto it = entIt.begin();
	auto *ent = (it != entIt.end()) ? *it : nullptr;
	auto plComponent = (ent != nullptr) ? ent->GetComponent<pragma::SPlayerComponent>() : pragma::ComponentHandle<pragma::SPlayerComponent>{};
	if(plComponent.expired() == false)
		kickTarget = plComponent.get();
	else
	{
		char *p;
		auto id = strtol(identifier.c_str(),&p,10);
		if(p != nullptr)
		{
			auto &players = pragma::SPlayerComponent::GetAll();
			if(id < players.size() && id >= 0)
				kickTarget = players.at(id);
		}
	}
	if(kickTarget == nullptr)
	{
		Con::cwar<<"WARNING: No player with id or name '"<<identifier<<"' found!"<<Con::endl;
		return;
	}
	std::string reason;
	if(argv.size() > 1)
		reason = argv[1];
	kickTarget->Kick(reason);
}

#ifdef _DEBUG
void CMD_sv_dump_netmessages(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	auto *map = GetServerMessageMap();
	std::unordered_map<std::string,unsigned int> *netmessages;
	map->GetNetMessages(&netmessages);
	if(!argv.empty())
	{
		auto id = atoi(argv.front().c_str());
		for(auto it=netmessages->begin();it!=netmessages->end();++it)
		{
			if(it->second == id)
			{
				Con::cout<<"Message Identifier: "<<it->first<<Con::endl;
				return;
			}
		}
		Con::cout<<"No message with id "<<id<<" found!"<<Con::endl;
		return;
	}
	for(auto it=netmessages->begin();it!=netmessages->end();++it)
		Con::cout<<it->first<<" = "<<it->second<<Con::endl;
}
#endif

DLLSERVER void CMD_entities_sv(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	if(!state->IsGameActive())
		return;
	auto sortedEnts = util::cmd::get_sorted_entities(*s_game,pl);
	std::optional<std::string> className = {};
	if(argv.empty() == false)
		className = '*' +argv.front() +'*';
	std::optional<std::string> modelName {};
	if(argv.size() > 1)
		modelName = '*' +argv[1] +'*';
	for(auto &pair : sortedEnts)
	{
		if(className.has_value() && ustring::match(pair.first->GetClass(),*className) == false)
			continue;
		if(modelName.has_value() && ustring::match(pair.first->GetModelName(),*modelName) == false)
			continue;
		Con::cout<<*pair.first<<Con::endl;
	}
}

void CMD_list_maps(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string>&)
{
	std::vector<std::string> files;
	auto exts = pragma::asset::get_supported_extensions(pragma::asset::Type::Map);
	for(auto &ext : exts)
		filemanager::find_files("maps/*." +ext,&files,nullptr);
	for(auto &f : files)
		ufile::remove_extension_from_filename(f,exts);
	std::sort(files.begin(),files.end());
	Con::cout<<"Available maps:"<<Con::endl;
	for(auto &f : files)
	{
		ufile::remove_extension_from_filename(f);
		Con::cout<<f<<Con::endl;
	}
	Con::cout<<Con::endl;
}

DLLSERVER void CMD_status_sv(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&)
{
	auto &players = pragma::SPlayerComponent::GetAll();
	std::string ip;
	auto *sv = server->GetServer();
	if(sv == nullptr)
	{
		std::stringstream str;
		str<<"[::1]";
		ip = str.str();
	}
	else
	{
		auto hostIp = sv->GetHostIP();
		ip = hostIp.has_value() ? *hostIp : "Unknown";
	}
	auto &serverData = server->GetServerData();
	Con::cout<<"hostname:\t"<<serverData.name<<Con::endl;
	Con::cout<<"udp/ip:\t\t"<<ip<<Con::endl;
	Con::cout<<"map:\t\t"<<serverData.map<<Con::endl;
	Con::cout<<"players:\t"<<players.size()<<" ("<<serverData.maxPlayers<<" max)"<<Con::endl<<Con::endl;
	Con::cout<<"#  userid\tname    \tconnected\tping";
	auto bServerRunning = server->IsServerRunning();
	if(bServerRunning == true)
		Con::cout<<"\tadr";
	Con::cout<<Con::endl;
	auto numPlayers = players.size();
	for(auto i=decltype(numPlayers){0};i<numPlayers;++i)
	{
		auto *pl = players.at(i);
		auto *session = pl->GetClientSession();
		auto nameC = pl->GetEntity().GetNameComponent();
		Con::cout<<"# \t"<<i<<"\t"<<"\""<<(nameC.valid() ? nameC->GetName() : "")<<"\""<<"\t"<<FormatTime(pl->TimeConnected())<<"     \t";
		if(session != nullptr)
			Con::cout<<session->GetLatency();
		else
			Con::cout<<"?";
		if(bServerRunning == true)
			Con::cout<<"\t"<<pl->GetClientIP();
		Con::cout<<Con::endl;
	}
}