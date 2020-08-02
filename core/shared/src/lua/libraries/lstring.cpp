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

uint32_t Lua::string::calc_levenshtein_distance(const std::string &s0,const std::string &s1) {return ustring::calc_levenshtein_distance(s0,s1);}
double Lua::string::calc_levenshtein_similarity(const std::string &s0,const std::string &s1) {return ustring::calc_levenshtein_similarity(s0,s1);}
void Lua::string::find_longest_common_substring(const std::string &s0,const std::string &s1,size_t &outStartIdx,size_t &outLen,size_t &outEndIdx)
{
	outLen = ustring::longest_common_substring(s0,s1,outStartIdx,outEndIdx);
}
void Lua::string::find_similar_elements(lua_State *l,const std::string &baseElement,luabind::table<> inElements,uint32_t limit,luabind::object &outSimilarElements,luabind::object &outSimilarities)
{
	auto numElements = Lua::GetObjectLength(l,2);
	std::vector<std::string> elements {};
	elements.reserve(numElements);
	for(auto it=luabind::iterator{inElements};it!=luabind::iterator{};++it)
	{
		auto val = luabind::object_cast_nothrow<std::string>(*it,std::string{});
		elements.push_back(val);
	}

	std::vector<size_t> similarElements {};
	std::vector<float> similarities {};
	ustring::gather_similar_elements(baseElement,elements,similarElements,limit,&similarities);

	uint32_t offset = 1u;
	outSimilarElements = luabind::newtable(l);
	for(auto idx : similarElements)
		outSimilarElements[offset++] = idx +1;

	offset = 1u;
	outSimilarities = luabind::newtable(l);
	for(auto &val : similarities)
		outSimilarities[offset++] = val;
}
std::string Lua::string::join(lua_State *l,luabind::table<> values,const std::string &joinChar)
{
	std::string r {};
	auto bFirst = true;
	for(auto it=luabind::iterator{values};it!=luabind::iterator{};++it)
	{
		auto val = luabind::object_cast_nothrow<std::string>(*it,std::string{});
		if(bFirst == false)
			r += joinChar;
		else
			bFirst = false;
		r += val;
	}
	return r;
}
std::vector<std::string> Lua::string::split(lua_State *l,const std::string &str,const std::string &delimiter)
{
	size_t len = delimiter.length();
	size_t from = 0;
	size_t f = str.find(delimiter,from);
	std::vector<std::string> result {};
	while(f != std::string::npos)
	{
		result.push_back(str.substr(from,f -from));

		from = f +len;
		f = str.find(delimiter,from);
	}
	result.push_back(str.substr(from));
	return result;
}
std::string Lua::string::remove_whitespace(const std::string &s)
{
	std::string str = s;
	ustring::remove_whitespace(str);
	return str;
}
std::string Lua::string::remove_quotes(const std::string &s)
{
	std::string str = s;
	ustring::remove_quotes(str);
	return str;
}
