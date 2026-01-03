// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.libraries.locale;
import :client_state;
import pragma.string.unicode;

void Lua::Locale::change_language(const std::string &lan) { pragma::locale::set_language(lan); }

void Lua::Locale::set_localization(const std::string &id, const std::string &text) { pragma::locale::set_localization(id, text); }

int Lua::Locale::get_text(lua::State *l)
{
	auto id = CheckString(l, 1);
	std::vector<std::string> args {};
	auto bReturnSuccess = false;
	auto argIdx = 2;
	if(IsSet(l, argIdx) && IsTable(l, argIdx)) {
		auto numArgs = GetObjectLength(l, argIdx);
		args.reserve(numArgs);
		for(auto i = decltype(numArgs) {0u}; i < numArgs; ++i) {
			PushInt(l, i + 1);
			GetTableValue(l, argIdx);
			args.push_back(CheckString(l, -1));

			Pop(l, 1);
		}
		++argIdx;
	}
	else if(IsNil(l, argIdx))
		++argIdx;
	if(IsSet(l, argIdx))
		bReturnSuccess = CheckBool(l, argIdx);
	uint32_t numResults = 1;
	if(bReturnSuccess == true) {
		std::string r;
		auto b = pragma::locale::get_text(id, args, r);
		PushBool(l, b);
		PushString(l, r);
		++numResults;
		return numResults;
	}
	PushString(l, pragma::locale::get_text(id, args));
	return numResults;
}

bool Lua::Locale::load(const std::string &fileName) { return pragma::locale::load(fileName) != pragma::locale::LoadResult::Failed; }

const std::string &Lua::Locale::get_language() { return pragma::locale::get_language(); }

int Lua::Locale::get_languages(lua::State *l)
{
	auto languages = pragma::locale::get_languages();
	auto t = CreateTable(l);
	for(auto &pair : languages) {
		PushString(l, pair.first);
		PushString(l, pair.second.displayName);
		SetTableValue(l, t);
	}
	return 1;
}

bool Lua::Locale::localize(const std::string &identifier, const std::string &lan, const std::string &category, const std::string &text) { return pragma::locale::localize(identifier, lan, category, text); }
bool Lua::Locale::relocalize(const std::string &identifier, const std::string &newIdentifier, const std::string &oldCategory, const std::string &newCategory) { return pragma::locale::relocalize(identifier, newIdentifier, oldCategory, newCategory); }

void Lua::Locale::clear() { pragma::locale::clear(); }
Lua::map<std::string, std::string> Lua::Locale::get_texts(lua::State *l)
{
	auto &texts = pragma::locale::get_texts();
	auto t = luabind::newtable(l);
	for(auto &pair : texts)
		t[pair.first] = pair.second.cpp_str();
	return t;
}
Lua::opt<Lua::map<std::string, std::string>> Lua::Locale::parse(lua::State *l, const std::string &fileName, const std::string &lan)
{
	std::unordered_map<std::string, pragma::string::Utf8String> texts;
	auto res = pragma::locale::parse_file(fileName, lan, texts);
	if(res != pragma::locale::LoadResult::Success)
#ifdef WINDOWS_CLANG_COMPILER_FIX
		return luabind::object {};
#else
		return nil;
#endif
	auto t = luabind::newtable(l);
	for(auto &pair : texts)
		t[pair.first] = pair.second.cpp_str();
	return t;
}
Lua::opt<Lua::map<std::string, std::string>> Lua::Locale::parse(lua::State *l, const std::string &fileName) { return parse(l, fileName, pragma::locale::get_language()); }
