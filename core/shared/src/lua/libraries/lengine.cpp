/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/lua/libraries/lengine.h"
#include "pragma/input/inputhelper.h"
#include <pragma/engine.h>
#include "pragma/engine_version.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/file_formats/wmd_load.h"
#include "pragma/util/util_module.hpp"
#include "luasystem.h"
#include "pragma/model/modelmesh.h"
#include <pragma/lua/lua_call.hpp>
#include <sharedutils/util_path.hpp>
#include <mathutil/color.h>

extern DLLNETWORK Engine *engine;

void Lua::engine::exit() { ::engine->ShutDown(); }

std::string Lua::engine::get_working_directory()
{
	auto path = ::util::Path::CreatePath(::util::get_program_path());
	return path.GetString();
}

Lua::tb<void> Lua::engine::get_info(lua_State *l)
{
	auto t = luabind::newtable(l);
	t["version"] = get_engine_version();
	t["prettyVersion"] = get_pretty_engine_version();
	t["identifier"] = engine_info::get_identifier();
	t["twitterURL"] = engine_info::get_twitter_url();
	t["redditURL"] = engine_info::get_reddit_url();
	t["discordURL"] = engine_info::get_discord_url();
	t["steamAppId"] = engine_info::get_steam_app_id();
	t["websiteURL"] = engine_info::get_website_url();
	t["wikiURL"] = engine_info::get_wiki_url();
	t["name"] = engine_info::get_name();
	return t;
}

Lua::opt<Lua::tb<void>> Lua::engine::get_git_info(lua_State *l)
{
	auto gitInfo = engine_info::get_git_info();
	if(!gitInfo.has_value())
		return Lua::nil;
	auto t = luabind::newtable(l);
	t["ref"] = gitInfo->ref;
	t["commitSha"] = gitInfo->commitSha;
	t["dateTime"] = gitInfo->dateTime;
	return t;
}

void Lua::engine::PrecacheModel_sv(lua_State *l, const std::string &mdlName)
{
	/*auto *nw = ::engine->GetNetworkState(l);
	FWMD wmd(nw->GetGameState());
	wmd.Load<Model,ModelMesh,ModelSubMesh>(
		nw->GetGameState(),mdlName,[nw](const std::string &mdlName) -> std::shared_ptr<Model> {
			return nw->GetGameState()->LoadModel(mdlName,false);
		}
	);*/
}

std::shared_ptr<Model> Lua::engine::get_model(lua_State *l, const std::string &mdlName)
{
	auto *state = ::engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	return game->LoadModel(mdlName);
}

void Lua::engine::LoadSoundScripts(lua_State *l, const std::string &fileName, bool precache)
{
	NetworkState *state = ::engine->GetNetworkState(l);
	state->LoadSoundScripts(fileName.c_str(), precache);
}
void Lua::engine::LoadSoundScripts(lua_State *l, const std::string &fileName) { LoadSoundScripts(l, fileName, false); }

bool Lua::engine::LibraryExists(lua_State *l, const std::string &library)
{
	auto libAbs = util::get_normalized_module_path(library, ::engine->GetNetworkState(l)->IsClient());
	return FileManager::Exists(libAbs);
}

Lua::var<bool, std::string> Lua::engine::LoadLibrary(lua_State *l, const std::string &path)
{
	NetworkState *state = ::engine->GetNetworkState(l);
	std::string err;
	bool b = state->InitializeLibrary(path, &err, l) != nullptr;
	if(b)
		return luabind::object {l, true};
	return luabind::object {l, err};
}

uint64_t Lua::engine::GetTickCount() { return ::engine->GetTickCount(); }

void Lua::engine::set_record_console_output(bool record) { ::engine->SetRecordConsoleOutput(record); }
Lua::opt<Lua::mult<std::string, Con::MessageFlags, Lua::opt<Color>>> Lua::engine::poll_console_output(lua_State *l)
{
	auto output = ::engine->PollConsoleOutput();
	if(output.has_value() == false)
		return nil;
	luabind::object color {};
	if(output->color)
		color = {l, *output->color};
	return Lua::mult<std::string, Con::MessageFlags, Lua::opt<Color>> {l, output->output, output->messageFlags, opt<Color> {color}};
}
