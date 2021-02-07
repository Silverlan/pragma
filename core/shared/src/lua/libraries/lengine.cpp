/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/lua/libraries/lengine.h"
#include "pragma/input/inputhelper.h"
#include <pragma/engine.h>
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/file_formats/wmd_load.h"
#include "luasystem.h"
#include "pragma/model/modelmesh.h"
#include <pragma/lua/lua_call.hpp>
#include <sharedutils/util_path.hpp>
#include <mathutil/color.h>

extern DLLNETWORK Engine *engine;

void Lua::engine::exit() {::engine->ShutDown();}

std::string Lua::engine::get_working_directory()
{
	auto path = ::util::Path::CreatePath(::util::get_program_path());
	return path.GetString();
}

int Lua::engine::get_info(lua_State *l)
{
	auto t = Lua::CreateTable(l);

	Lua::PushString(l,"identifier");
	Lua::PushString(l,engine_info::get_identifier());
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"patreonURL");
	Lua::PushString(l,engine_info::get_patreon_url());
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"patrons");
	auto tPatrons = Lua::CreateTable(l);
	auto patrons = engine_info::get_patrons();
	int32_t idx = 1;
	for(auto &patron : patrons)
	{
		Lua::PushInt(l,idx++);
		Lua::PushString(l,patron);
		Lua::SetTableValue(l,tPatrons);
	}
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"totalPatronCount");
	Lua::PushInt(l,engine_info::get_total_patron_count());
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"twitterURL");
	Lua::PushString(l,engine_info::get_twitter_url());
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"redditURL");
	Lua::PushString(l,engine_info::get_reddit_url());
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"discordURL");
	Lua::PushString(l,engine_info::get_discord_url());
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"steamAppId");
	Lua::PushInt(l,engine_info::get_steam_app_id());
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"websiteURL");
	Lua::PushString(l,engine_info::get_website_url());
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"wikiURL");
	Lua::PushString(l,engine_info::get_wiki_url());
	Lua::SetTableValue(l,t);

	Lua::PushString(l,"name");
	Lua::PushString(l,engine_info::get_name());
	Lua::SetTableValue(l,t);
	
	return 1;
}

void Lua::engine::PrecacheModel_sv(lua_State *l,const std::string &mdlName)
{
	auto *nw = ::engine->GetNetworkState(l);
	FWMD wmd(nw->GetGameState());
	wmd.Load<Model,ModelMesh,ModelSubMesh>(
		nw->GetGameState(),mdlName,[nw](const std::string &matName,bool bReload) -> Material* {
			return nw->LoadMaterial(matName,bReload);
		},[nw](const std::string &mdlName) -> std::shared_ptr<Model> {
			return nw->GetGameState()->LoadModel(mdlName,false);
		}
	);
}

std::shared_ptr<Model> Lua::engine::get_model(lua_State *l,const std::string &mdlName)
{
	auto *state = ::engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	return game->LoadModel(mdlName);
}

void Lua::engine::LoadSoundScripts(lua_State *l,const std::string &fileName,bool precache)
{
	NetworkState *state = ::engine->GetNetworkState(l);
	state->LoadSoundScripts(fileName.c_str(),precache);
}
void Lua::engine::LoadSoundScripts(lua_State *l,const std::string &fileName) {LoadSoundScripts(l,fileName,false);}

int Lua::engine::LoadLibrary(lua_State *l)
{
	std::string path = Lua::CheckString(l,1);
	NetworkState *state = ::engine->GetNetworkState(l);
	std::string err;
	bool b = state->InitializeLibrary(path,&err,l) != nullptr;
	if(b == true)
		Lua::PushBool(l,b);
	else
		Lua::PushString(l,err);
	return 1;
}

uint64_t Lua::engine::GetTickCount() {return ::engine->GetTickCount();}

void Lua::engine::set_record_console_output(bool record) {::engine->SetRecordConsoleOutput(record);}
int32_t Lua::engine::poll_console_output(lua_State *l)
{
	auto output = ::engine->PollConsoleOutput();
	if(output.has_value() == false)
		return 0;
	Lua::PushString(l,output->output);
	Lua::PushInt(l,umath::to_integral(output->messageFlags));
	if(output->color)
	{
		Lua::Push<Color>(l,*output->color);
		return 3;
	}
	return 2;
}
