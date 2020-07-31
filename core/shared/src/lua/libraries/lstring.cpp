/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/lua/libraries/lstring.hpp"
#include <pragma/lua/luaapi.h>

int32_t Lua::string::is_integer(lua_State *l)
{
	Lua::PushBool(l,ustring::is_integer(Lua::CheckString(l,1)));
	return 1;
}
int32_t Lua::string::is_number(lua_State *l)
{
	Lua::PushBool(l,ustring::is_number(Lua::CheckString(l,1)));
	return 1;
}
int32_t Lua::string::calc_levenshtein_distance(lua_State *l)
{
	auto *str1 = Lua::CheckString(l,1);
	auto *str2 = Lua::CheckString(l,2);
	auto dist = ustring::calc_levenshtein_distance(str1,str2);
	Lua::PushInt(l,dist);
	return 1;
}
int32_t Lua::string::calc_levenshtein_similarity(lua_State *l)
{
	auto *str1 = Lua::CheckString(l,1);
	auto *str2 = Lua::CheckString(l,2);
	auto sim = ustring::calc_levenshtein_similarity(str1,str2);
	Lua::PushNumber(l,sim);
	return 1;
}
int32_t Lua::string::find_longest_common_substring(lua_State *l)
{
	auto *str1 = Lua::CheckString(l,1);
	auto *str2 = Lua::CheckString(l,2);
	std::size_t startIdx = 0;
	std::size_t endIdx = 0;
	auto len = ustring::longest_common_substring(str1,str2,startIdx,endIdx);
	Lua::PushInt(l,startIdx);
	Lua::PushInt(l,len);
	Lua::PushInt(l,endIdx);
	return 3;
}
int32_t Lua::string::find_similar_elements(lua_State *l)
{
	std::string baseElement = Lua::CheckString(l,1);
	auto tElements = 2;
	Lua::CheckTable(l,tElements);
	auto limit = Lua::CheckInt(l,3);
	auto numElements = Lua::GetObjectLength(l,tElements);
	std::vector<std::string> elements {};
	elements.reserve(numElements);
	for(auto i=decltype(numElements){0u};i<numElements;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,tElements);
		elements.push_back(Lua::CheckString(l,-1));
		Lua::Pop(l,1);
	}

	std::vector<size_t> similarElements {};
	std::vector<float> similarities {};
	uint32_t offset = 0u;
	ustring::gather_similar_elements(baseElement,elements,similarElements,limit,&similarities);

	auto tOutElements = Lua::CreateTable(l);
	offset = 1u;
	for(auto idx : similarElements)
	{
		Lua::PushInt(l,offset++);
		Lua::PushInt(l,idx +1);
		Lua::SetTableValue(l,tOutElements);
	}

	auto tSimilarities = Lua::CreateTable(l);
	offset = 1u;
	for(auto &val : similarities)
	{
		Lua::PushInt(l,offset++);
		Lua::PushNumber(l,val);
		Lua::SetTableValue(l,tSimilarities);
	}
	return 2;
}
int32_t Lua::string::join(lua_State *l)
{
	int32_t t = 1;
	Lua::CheckTable(l,t);

	auto joinChar = ';';
	if(Lua::IsSet(l,2))
		joinChar = *Lua::CheckString(l,2);

	std::string r {};
	auto numEls = Lua::GetObjectLength(l,t);
	auto bFirst = true;
	for(auto i=decltype(numEls){0};i<numEls;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,t);
		auto *v = Lua::CheckString(l,-1);
		if(bFirst == false)
			r += joinChar;
		else
			bFirst = false;
		r += v;
		Lua::Pop(l,1);
	}
	Lua::PushString(l,r);
	return 1;
}
int32_t Lua::string::split(lua_State *l)
{
	std::string str(Lua::CheckString(l,1));
	const char *delimiter = Lua::CheckString(l,2);
	lua_newtable(l);
	int top = lua_gettop(l);
	int n = 1;

	size_t len = strlen(delimiter);
	size_t from = 0;
	size_t f = str.find(delimiter,from);
	while(f != std::string::npos)
	{
		lua_pushstring(l,str.substr(from,f -from).c_str());
		lua_rawseti(l,top,n);
		n++;

		from = f +len;
		f = str.find(delimiter,from);
	}
	lua_pushstring(l,str.substr(from).c_str());
	lua_rawseti(l,top,n);
	n++;
	return 1;
}
int32_t Lua::string::remove_whitespace(lua_State *l)
{
	std::string str = Lua::CheckString(l,1);
	ustring::remove_whitespace(str);
	Lua::PushString(l,str);
	return 1;
}
int32_t Lua::string::remove_quotes(lua_State *l)
{
	std::string str = Lua::CheckString(l,1);
	ustring::remove_quotes(str);
	Lua::PushString(l,str);
	return 1;
}
