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
#include <mathutil/color.h>

extern DLLENGINE Engine *engine;

int Lua::engine::CreateLight(lua_State *l)
{
	Vector3 *pos = _lua_Vector_check(l,1);
	UNUSED(pos);
	float power = Lua::CheckNumber<float>(l,2);
	UNUSED(power);
	Vector3 *col = _lua_Vector_check(l,3);
	UNUSED(col);
	//engine->CreateLight(*pos,power,*col); // WEAVETODO
	return 0;
}

int Lua::engine::exit(lua_State *l)
{
	::engine->ShutDown();
	return 0;
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

int Lua::engine::RemoveLights(lua_State*)
{
	//engine->RemoveLights(); // WEAVETODO
	return 0;
}

int Lua::engine::CreateSprite(lua_State*)
{
	/*float scale = luaL_checknumber(l,1);
	Vector3 *pos = _lua_Vector_check(l,2);
	Sprite *spr = engine->CreateSprite(scale);
	spr->SetPosition(pos);*/ // WEAVETODO
	return 0;
}

int Lua::engine::PrecacheModel_sv(lua_State *l)
{
	std::string mdl = luaL_checkstring(l,1);
	auto *nw = ::engine->GetNetworkState(l);
	FWMD wmd(nw->GetGameState());
	wmd.Load<Model,ModelMesh,ModelSubMesh>(
		nw->GetGameState(),mdl.c_str(),[nw](const std::string &matName,bool bReload) -> Material* {
			return nw->LoadMaterial(matName,bReload);
		},[nw](const std::string &mdlName) -> std::shared_ptr<Model> {
			return nw->GetGameState()->LoadModel(mdlName,false);
		}
	);
	return 0;
}

int Lua::engine::get_model(lua_State *l)
{
	auto *state = ::engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *name = Lua::CheckString(l,1);
	auto mdl = game->LoadModel(name);
	if(mdl == nullptr)
		return 0;
	Lua::Push<decltype(mdl)>(l,mdl);
	return 1;
}

int Lua::engine::LoadSoundScripts(lua_State *l)
{
	NetworkState *state = ::engine->GetNetworkState(l);
	std::string file = luaL_checkstring(l,1);
	bool bPrecache = false;
	if(Lua::IsSet(l,2))
		bPrecache = Lua::CheckBool(l,2);
	state->LoadSoundScripts(file.c_str(),bPrecache);
	return 0;
}

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

int Lua::engine::GetTickCount(lua_State *l)
{
	auto tick = ::engine->GetTickCount();
	Lua::PushInt(l,tick);
	return 1;
}

int32_t Lua::engine::set_record_console_output(lua_State *l)
{
	auto record = Lua::CheckBool(l,1);
	::engine->SetRecordConsoleOutput(record);
	return 0;
}
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
