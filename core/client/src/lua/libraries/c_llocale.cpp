#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/lua/libraries/c_llocale.h"
#include "pragma/localization.h"

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

int Lua::Locale::get_text(lua_State *l)
{
	auto id = Lua::CheckString(l,1);
	auto bReturnSuccess = false;
	if(Lua::IsSet(l,2))
		bReturnSuccess = Lua::CheckBool(l,2);
	std::string r;
	auto b = ::Locale::GetText(id,r);
	uint32_t numResults = 1;
	if(bReturnSuccess == true)
	{
		Lua::PushBool(l,b);
		++numResults;
	}
	Lua::PushString(l,r);
	return numResults;
}

int Lua::Locale::load(lua_State *l)
{
	auto *fName = Lua::CheckString(l,1);
	Lua::PushBool(l,::Locale::Load(fName));
	return 1;
}

int Lua::Locale::get_language(lua_State *l)
{
	auto &lan = ::Locale::GetLanguage();
	Lua::PushString(l,lan);
	return 1;
}
