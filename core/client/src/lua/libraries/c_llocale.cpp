/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/lua/libraries/c_llocale.h"
#include "pragma/localization.h"

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

int Lua::Locale::change_language(lua_State *l)
{
	std::string lan = Lua::CheckString(l,1);
	::Locale::SetLanguage(lan);
	return 0;
}

int Lua::Locale::get_text(lua_State *l)
{
	auto id = Lua::CheckString(l,1);
	std::vector<std::string> args {};
	auto bReturnSuccess = false;
	auto argIdx = 2;
	if(Lua::IsSet(l,argIdx) && Lua::IsTable(l,argIdx))
	{
		auto numArgs = Lua::GetObjectLength(l,argIdx);
		args.reserve(numArgs);
		for(auto i=decltype(numArgs){0u};i<numArgs;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,argIdx);
			args.push_back(Lua::CheckString(l,-1));

			Lua::Pop(l,1);
		}
		++argIdx;
	}
	if(Lua::IsSet(l,argIdx))
		bReturnSuccess = Lua::CheckBool(l,argIdx);
	uint32_t numResults = 1;
	if(bReturnSuccess == true)
	{
		std::string r;
		auto b = ::Locale::GetText(id,args,r);
		Lua::PushBool(l,b);
		Lua::PushString(l,r);
		++numResults;
		return numResults;
	}
	Lua::PushString(l,::Locale::GetText(id,args));
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

int Lua::Locale::get_languages(lua_State *l)
{
	auto languages = ::Locale::GetLanguages();
	auto t = Lua::CreateTable(l);
	for(auto &pair : languages)
	{
		Lua::PushString(l,pair.first);
		Lua::PushString(l,pair.second);
		Lua::SetTableValue(l,t);
	}
	return 1;
}
