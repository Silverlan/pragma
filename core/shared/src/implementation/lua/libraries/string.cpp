// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.string;

std::string Lua::string::snake_case_to_camel_case(const std::string &str)
{
	auto newStr = str;
	bool nextCharIsUpper = false;
	for(int i = 0; i < newStr.length(); i++) {
		if(newStr[i] == '_') {
			nextCharIsUpper = true;
			continue;
		}
		if(nextCharIsUpper) {
			newStr[i] = toupper(newStr[i]);
			nextCharIsUpper = false;
		}
	}
	// Remove '_' from the string
	newStr.erase(remove(newStr.begin(), newStr.end(), '_'), newStr.end());
	return newStr;
}
std::string Lua::string::camel_case_to_snake_case(const std::string &str)
{
	auto newStr = str;
	for(int i = 0; i < newStr.length(); i++) {
		if(isupper(newStr[i])) {
			newStr.insert(i, "_");
			newStr[i + 1] = tolower(newStr[i + 1]);
		}
	}
	return newStr;
}
uint32_t Lua::string::calc_levenshtein_distance(const std::string &s0, const std::string &s1) { return pragma::string::calc_levenshtein_distance(s0, s1); }
double Lua::string::calc_levenshtein_similarity(const std::string &s0, const std::string &s1) { return pragma::string::calc_levenshtein_similarity(s0, s1); }
void Lua::string::find_longest_common_substring(const std::string &s0, const std::string &s1, size_t &outStartIdx, size_t &outLen, size_t &outEndIdx) { outLen = pragma::string::longest_common_substring(s0, s1, outStartIdx, outEndIdx); }
void Lua::string::find_similar_elements(lua::State *l, const std::string &baseElement, luabind::table<> inElements, uint32_t limit, luabind::object &outSimilarElements, luabind::object &outSimilarities)
{
	auto numElements = GetObjectLength(l, 2);
	std::vector<std::string> elements {};
	elements.reserve(numElements);
	for(auto it = luabind::iterator {inElements}; it != luabind::iterator {}; ++it) {
		auto val = luabind::object_cast_nothrow<std::string>(*it, std::string {});
		elements.push_back(val);
	}

	std::vector<size_t> similarElements {};
	std::vector<float> similarities {};
	pragma::string::gather_similar_elements(baseElement, elements, similarElements, limit, &similarities);

	uint32_t offset = 1u;
	outSimilarElements = luabind::newtable(l);
	for(auto idx : similarElements)
		outSimilarElements[offset++] = idx + 1;

	offset = 1u;
	outSimilarities = luabind::newtable(l);
	for(auto &val : similarities)
		outSimilarities[offset++] = val;
}
std::string Lua::string::join(lua::State *l, luabind::table<> values, const std::string &joinChar)
{
	std::string r {};
	auto bFirst = true;
	for(auto it = luabind::iterator {values}; it != luabind::iterator {}; ++it) {
		auto val = luabind::object_cast_nothrow<std::string>(*it, std::string {});
		if(bFirst == false)
			r += joinChar;
		else
			bFirst = false;
		r += val;
	}
	return r;
}
luabind::object Lua::string::split(lua::State *l, const std::string &str, const std::string &delimiter)
{
	if(delimiter.empty())
		return luabind::newtable(l);
	size_t len = delimiter.length();
	size_t from = 0;
	size_t f = str.find(delimiter, from);
	uint32_t idx = 1;
	auto result = luabind::newtable(l);
	while(f != std::string::npos) {
		result[idx++] = str.substr(from, f - from);

		from = f + len;
		f = str.find(delimiter, from);
	}
	result[idx] = str.substr(from);
	return result;
}
std::string Lua::string::remove_whitespace(const std::string &s)
{
	std::string str = s;
	pragma::string::remove_whitespace(str);
	return str;
}
std::string Lua::string::remove_quotes(const std::string &s)
{
	std::string str = s;
	pragma::string::remove_quotes(str);
	return str;
}
