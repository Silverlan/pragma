
#include "algorithm"

#include "pragma/lua/luaapi.h"

#include "sharedutils/util_string.h"

// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include <sharedutils/util.h>
#include <sharedutils/util_file.h>
#include <luabind/exception_handler.hpp>
#include <stack>

// import pragma.scripting.lua;

import pragma.shared;

static void print_lua_error_message(lua_State *l, const std::string &msg) { Con::cerr << Lua::GetErrorMessagePrefix(l) << Con::prefix << Con::PREFIX_LUA << Con::prefix << msg << Con::endl; }

static auto cvOpenEditorOnError = GetConVar("lua_open_editor_on_error");
std::optional<std::string> Lua::GetLuaFilePath(const std::string &fname)
{
	std::string fullLocalPath;
	if(FileManager::FindLocalPath(fname, fullLocalPath) == false)
		fullLocalPath = fname;
	filemanager::find_absolute_path(fullLocalPath, fullLocalPath);
	if(FileManager::ExistsSystem(fullLocalPath) == false)
		return {};
	return fullLocalPath;
}
void Lua::OpenFileInZeroBrane(const std::string &fname, uint32_t lineId)
{
	struct ZBFileInfo {
		ZBFileInfo(const std::string &fileName, uint32_t lineId) : fileName(fileName), lineId(lineId), tp(std::chrono::high_resolution_clock::now()) {}
		std::string fileName;
		uint32_t lineId;
		std::chrono::high_resolution_clock::time_point tp;
	};
	static std::vector<ZBFileInfo> zbFileInfo;
	static std::chrono::high_resolution_clock::time_point tLastFileOpened {};
	auto t = std::chrono::high_resolution_clock::now();
	if(std::chrono::duration_cast<std::chrono::seconds>(t - tLastFileOpened).count() > 5) {
		for(auto it = zbFileInfo.begin(); it != zbFileInfo.end();) {
			auto &info = *it;
			if(std::chrono::duration_cast<std::chrono::seconds>(t - info.tp).count() > 90)
				it = zbFileInfo.erase(it);
			else
				++it;
		}
		auto it = std::find_if(zbFileInfo.begin(), zbFileInfo.end(), [&fname, lineId](const ZBFileInfo &info) { return ustring::compare(info.fileName, fname, false) && info.lineId == lineId; });
		if(it == zbFileInfo.end()) {
			if(it != zbFileInfo.end())
				zbFileInfo.erase(it);
			zbFileInfo.push_back({fname, lineId});

			tLastFileOpened = t;
			auto fullLocalPath = GetLuaFilePath(fname);
			if(fullLocalPath.has_value())
				pragma::debug::open_file_in_zerobrane(*fullLocalPath, lineId);
		}
	}
}

const auto maxLuaPathLen = 120u;
static void strip_path_until_lua_dir(std::string &shortSrc)
{
	auto c = FileManager::GetDirectorySeparator();
	auto br = shortSrc.find(c);
	uint32_t offset = 0;
	auto bFound = false;
	auto luaPath = Lua::SCRIPT_DIRECTORY + c;
	while(br != std::string::npos && shortSrc.length() >= offset + luaPath.length() && (bFound = ustring::compare(shortSrc.data() + offset, luaPath.c_str(), false, luaPath.length())) == false) {
		offset = br + 1;
		br = shortSrc.find(c, br + 1);
	}
	if(bFound == false)
		return;
	shortSrc = ustring::substr(shortSrc, offset + luaPath.length());
	if(shortSrc.length() > maxLuaPathLen)
		shortSrc = "..." + shortSrc.substr(shortSrc.size() - maxLuaPathLen);
}

static void transform_path(const lua_Debug &d, std::string &errPath, int32_t currentLine)
{
	auto start = errPath.find("[string \"");
	if(start == std::string::npos)
		return;
	auto end = errPath.find_first_of(']', start);
	if(end == std::string::npos)
		return;
	auto qt0 = errPath.find_first_of('\"', start);
	auto qt1 = errPath.find_first_of('\"', qt0 + 1);
	if(qt0 < end && qt1 < end) {
		auto path = FileManager::GetCanonicalizedPath(Lua::get_source(d));
		filemanager::find_relative_path(path, path);
		strip_path_until_lua_dir(path);
		if(path.length() > maxLuaPathLen)
			path = "..." + path.substr(path.size() - maxLuaPathLen);

		if(Lua::GetLuaFilePath(Lua::SCRIPT_DIRECTORY_SLASH + path))
			path = pragma::scripting::lua::util::make_clickable_lua_script_link(path, currentLine);
		errPath = ustring::substr(errPath, 0, qt0 + 1) + path + ustring::substr(errPath, qt1);
	}
}

bool Lua::get_callstack(lua_State *l, std::stringstream &ss)
{
	int32_t level = 1;
	lua_Debug d {};
	auto r = lua_getstack(l, level, &d);
	if(r == 0)
		return false;
	while(r == 1) {
		if(lua_getinfo(l, "Sln", &d) != 0) {
			std::string t(level * 4, ' ');
			if(level >= 10) {
				ss << "\n" << t << "...";
				break;
			}
			else {
				auto filename = pragma::scripting::lua::util::make_clickable_lua_script_link(get_source(d), d.currentline);
				ss << "\n" << t << level << ": " << (d.name != nullptr ? d.name : "?") << "[" << d.linedefined << ":" << d.lastlinedefined << "] [" << d.what << ":" << d.namewhat << "] : " << filename;
			}
		}
		++level;
		r = lua_getstack(l, level, &d);
	}
	return true;
}

bool Lua::PrintTraceback(lua_State *l, std::stringstream &ssOut, const std::string *pOptErrMsg, std::string *optOutFormattedErrMsg)
{
	lua_Debug d {};
	int32_t level = 1;
	auto bFoundSrc = false;
	auto bNl = false;
	while(bFoundSrc == false && lua_getstack(l, level, &d) == 1) {
		if(lua_getinfo(l, "Sln", &d) != 0 && (strcmp(d.what, "Lua") == 0 || strcmp(d.what, "main") == 0)) {
			bFoundSrc = true;
			break;
		}
		++level;
	}

	auto errMsg = pOptErrMsg ? *pOptErrMsg : "";
	auto hasMsg = true;
	if(bFoundSrc == true) {
		if(!errMsg.empty() && errMsg.front() != '[') {
			std::string shortSrc = get_source(d);
			auto c = FileManager::GetDirectorySeparator();
			auto br = shortSrc.find(c);
			uint32_t offset = 0;
			auto bFound = false;
			auto luaPath = Lua::SCRIPT_DIRECTORY + c;
			while(br != std::string::npos && shortSrc.length() >= offset + luaPath.length() && (bFound = ustring::compare(shortSrc.data() + offset, luaPath.c_str(), false, luaPath.length())) == false) {
				offset = br + 1;
				br = shortSrc.find(c, br + 1);
			}
			if(bFound == true) {
				shortSrc = ustring::substr(shortSrc, offset);
				if(shortSrc.length() > maxLuaPathLen)
					shortSrc = "..." + shortSrc.substr(shortSrc.size() - maxLuaPathLen);
				shortSrc = "[string \"" + shortSrc + "\"]";
				//open_lua_file(fname,lineId);
			}
			std::stringstream ssErrMsg;
			auto lineMsg = pragma::scripting::lua::util::make_clickable_lua_script_link(shortSrc, d.currentline);
			ssErrMsg << lineMsg << " " << errMsg;
			errMsg = ssErrMsg.str();
		}
		transform_path(d, errMsg, d.currentline);
		ssOut << errMsg;
		bNl = pragma::scripting::lua::util::get_code_snippet(ssOut, get_source(d), d.currentline, ":");
	}
	else {
		ssOut << errMsg;
		hasMsg = !errMsg.empty();
	}
	if(optOutFormattedErrMsg)
		*optOutFormattedErrMsg = std::move(errMsg);
	// if(level != 1)
	{
		level = 1;
		if(bNl == true)
			ssOut << "\n\n";
		else
			ssOut << ":\n";
		ssOut << "    Callstack:";
		if(get_callstack(l, ssOut))
			hasMsg = true;
	}
	return hasMsg;
}

void Lua::PrintTraceback(lua_State *l, const std::string *pOptErrMsg)
{
	std::stringstream ssTbMsg;
	Lua::PrintTraceback(l, ssTbMsg, pOptErrMsg);
	auto tbMsg = ssTbMsg.str();

	std::stringstream ss;
	pragma::scripting::lua::util::get_lua_doc_info(ss, tbMsg);
	Con::cout << ss.str();
	Con::flush();
}

int Lua::HandleTracebackError(lua_State *l)
{
	if(!Lua::IsString(l, -1))
		return 1;
	std::string msg = Lua::ToString(l, -1);
	auto *nw = pragma::get_engine()->GetNetworkState(l);
	auto *game = nw ? nw->GetGameState() : nullptr;
	if(game)
		game->CallLuaCallbacks<void, std::string>("OnLuaError", msg);
	Lua::PrintTraceback(l, &msg);
	return 1;
}

static std::optional<std::string> format_syntax_error(const std::string &msg, Lua::StatusCode r, const std::string *optFilename)
{
	if(r != Lua::StatusCode::ErrorSyntax && r != Lua::StatusCode::ErrorFile)
		return {};
	auto errInfo = pragma::scripting::lua::util::parse_syntax_error_message(msg);
	if(!errInfo)
		return msg;
	std::stringstream ssMsg;
	pragma::scripting::lua::util::get_code_snippet(ssMsg, optFilename ? *optFilename : errInfo->first, errInfo->second, ":");
	return ssMsg.str();
}

static void handle_syntax_error(lua_State *l, Lua::StatusCode r, const std::string *fileName)
{
	if(r != Lua::StatusCode::ErrorSyntax && r != Lua::StatusCode::ErrorFile)
		return;
	if(!Lua::IsString(l, -1))
		return;
	std::string err = Lua::ToString(l, -1);
	auto msg = format_syntax_error(err, r, fileName);
	if(!msg)
		return;
	print_lua_error_message(l, *msg);
}

void Lua::HandleSyntaxError(lua_State *l, Lua::StatusCode r, const std::string &fileName) { handle_syntax_error(l, r, &fileName); }

void Lua::HandleSyntaxError(lua_State *l, Lua::StatusCode r) { handle_syntax_error(l, r, nullptr); }

void Lua::initialize_error_handler()
{
	luabind::register_exception_handler<Lua::Exception>(+[](lua_State *L, const Lua::Exception &e) { lua_pushstring(L, e.what()); });
	luabind::set_pcall_callback([](lua_State *l) -> void {
		Lua::PushCFunction(l, [](lua_State *l) -> int32_t {
			if(Lua::IsString(l, -1) == false)
				return 0; // This should never happen
			std::string errMsg = Lua::CheckString(l, -1);
			auto formattedMsg = pragma::scripting::lua::format_error_message(l, errMsg, Lua::StatusCode::ErrorRun, nullptr);
			pragma::scripting::lua::submit_error(l, formattedMsg);
			return 0;
		});
	});
}
