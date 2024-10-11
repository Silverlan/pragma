/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"

namespace pragma::string {
	class Utf8String;
	class Utf8StringView;
	class Utf8StringArg;
};

#include "pragma/clientstate/clientstate.h"
#include "pragma/lua/libraries/c_llocale.h"
#include "pragma/localization.h"

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

void Lua::Locale::change_language(const std::string &lan) { ::Locale::SetLanguage(lan); }

void Lua::Locale::set_localization(const std::string &id, const std::string &text) { ::Locale::SetLocalization(id, text); }

int Lua::Locale::get_text(lua_State *l)
{
	auto id = Lua::CheckString(l, 1);
	std::vector<std::string> args {};
	auto bReturnSuccess = false;
	auto argIdx = 2;
	if(Lua::IsSet(l, argIdx) && Lua::IsTable(l, argIdx)) {
		auto numArgs = Lua::GetObjectLength(l, argIdx);
		args.reserve(numArgs);
		for(auto i = decltype(numArgs) {0u}; i < numArgs; ++i) {
			Lua::PushInt(l, i + 1);
			Lua::GetTableValue(l, argIdx);
			args.push_back(Lua::CheckString(l, -1));

			Lua::Pop(l, 1);
		}
		++argIdx;
	}
	else if(Lua::IsNil(l, argIdx))
		++argIdx;
	if(Lua::IsSet(l, argIdx))
		bReturnSuccess = Lua::CheckBool(l, argIdx);
	uint32_t numResults = 1;
	if(bReturnSuccess == true) {
		std::string r;
		auto b = ::Locale::GetText(id, args, r);
		Lua::PushBool(l, b);
		Lua::PushString(l, r);
		++numResults;
		return numResults;
	}
	Lua::PushString(l, ::Locale::GetText(id, args));
	return numResults;
}

bool Lua::Locale::load(const std::string &fileName) { return ::Locale::Load(fileName) != ::Locale::LoadResult::Failed; }

const std::string &Lua::Locale::get_language() { return ::Locale::GetLanguage(); }

int Lua::Locale::get_languages(lua_State *l)
{
	auto languages = ::Locale::GetLanguages();
	auto t = Lua::CreateTable(l);
	for(auto &pair : languages) {
		Lua::PushString(l, pair.first);
		Lua::PushString(l, pair.second.displayName);
		Lua::SetTableValue(l, t);
	}
	return 1;
}

bool Lua::Locale::localize(const std::string &identifier, const std::string &lan, const std::string &category, const std::string &text) { return ::Locale::Localize(identifier, lan, category, text); }

void Lua::Locale::clear() { ::Locale::Clear(); }
Lua::map<std::string, std::string> Lua::Locale::get_texts(lua_State *l)
{
	auto &texts = ::Locale::GetTexts();
	auto t = luabind::newtable(l);
	for(auto &pair : texts)
		t[pair.first] = pair.second.cpp_str();
	return t;
}
Lua::opt<Lua::map<std::string, std::string>> Lua::Locale::parse(lua_State *l, const std::string &fileName, const std::string &lan)
{
	std::unordered_map<std::string, pragma::string::Utf8String> texts;
	auto res = ::Locale::ParseFile(fileName, lan, texts);
	if(res != ::Locale::LoadResult::Success)
		return Lua::nil;
	auto t = luabind::newtable(l);
	for(auto &pair : texts)
		t[pair.first] = pair.second.cpp_str();
	return t;
}
Lua::opt<Lua::map<std::string, std::string>> Lua::Locale::parse(lua_State *l, const std::string &fileName) { return parse(l, fileName, ::Locale::GetLanguage()); }
