#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/lua/libraries/lengine.h"
#include "pragma/input/inputhelper.h"
#include <pragma/engine.h>
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/file_formats/wmd_load.h"
#include "luasystem.h"
#include "pragma/model/modelmesh.h"
extern DLLENGINE Engine *engine;
DLLNETWORK int Lua_engine_CreateLight(lua_State *l)
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

DLLNETWORK int Lua_engine_RemoveLights(lua_State*)
{
	//engine->RemoveLights(); // WEAVETODO
	return 0;
}

DLLNETWORK int Lua_engine_CreateSprite(lua_State*)
{
	/*float scale = luaL_checknumber(l,1);
	Vector3 *pos = _lua_Vector_check(l,2);
	Sprite *spr = engine->CreateSprite(scale);
	spr->SetPosition(pos);*/ // WEAVETODO
	return 0;
}

DLLNETWORK int Lua_engine_PrecacheModel_sv(lua_State *l)
{
	std::string mdl = luaL_checkstring(l,1);
	auto *nw = engine->GetNetworkState(l);
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

DLLNETWORK int Lua_engine_LoadSoundScripts(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	std::string file = luaL_checkstring(l,1);
	bool bPrecache = false;
	if(Lua::IsSet(l,2))
		bPrecache = Lua::CheckBool(l,2);
	state->LoadSoundScripts(file.c_str(),bPrecache);
	return 0;
}

DLLNETWORK int Lua_engine_LoadLibrary(lua_State *l)
{
	std::string path = Lua::CheckString(l,1);
	NetworkState *state = engine->GetNetworkState(l);
	std::string err;
	bool b = state->InitializeLibrary(path,&err,l) != nullptr;
	if(b == true)
		Lua::PushBool(l,b);
	else
		Lua::PushString(l,err);
	return 1;
}

int Lua_engine_GetTickCount(lua_State *l)
{
	auto tick = engine->GetTickCount();
	Lua::PushInt(l,tick);
	return 1;
}