// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.error_handling;

static void print_lua_error_message(lua::State *l, const std::string &msg) { Con::CERR << Lua::GetErrorMessagePrefix(l) << Con::prefix << Con::PREFIX_LUA << Con::prefix << msg << Con::endl; }

static auto cvOpenEditorOnError = pragma::console::get_con_var("lua_open_editor_on_error");
std::optional<std::string> Lua::GetLuaFilePath(const std::string &fname)
{
	std::string fullLocalPath;
	if(pragma::fs::find_local_path(fname, fullLocalPath) == false)
		fullLocalPath = fname;
	pragma::fs::find_absolute_path(fullLocalPath, fullLocalPath);
	if(pragma::fs::exists_system(fullLocalPath) == false)
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
		auto it = std::find_if(zbFileInfo.begin(), zbFileInfo.end(), [&fname, lineId](const ZBFileInfo &info) { return pragma::string::compare(info.fileName, fname, false) && info.lineId == lineId; });
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
	auto c = pragma::fs::get_directory_separator();
	auto br = shortSrc.find(c);
	uint32_t offset = 0;
	auto bFound = false;
	auto luaPath = Lua::SCRIPT_DIRECTORY + c;
	while(br != std::string::npos && shortSrc.length() >= offset + luaPath.length() && (bFound = pragma::string::compare(shortSrc.data() + offset, luaPath.c_str(), false, luaPath.length())) == false) {
		offset = br + 1;
		br = shortSrc.find(c, br + 1);
	}
	if(bFound == false)
		return;
	shortSrc = pragma::string::substr(shortSrc, offset + luaPath.length());
	if(shortSrc.length() > maxLuaPathLen)
		shortSrc = "..." + shortSrc.substr(shortSrc.size() - maxLuaPathLen);
}

static void transform_path(const lua::DebugInfo &d, std::string &errPath, int32_t currentLine)
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
		auto path = pragma::fs::get_canonicalized_path(Lua::get_source(d));
		pragma::fs::find_relative_path(path, path);
		strip_path_until_lua_dir(path);
		if(path.length() > maxLuaPathLen)
			path = "..." + path.substr(path.size() - maxLuaPathLen);

		if(Lua::GetLuaFilePath(Lua::SCRIPT_DIRECTORY_SLASH + path))
			path = pragma::scripting::lua_core::util::make_clickable_lua_script_link(path, currentLine);
		errPath = pragma::string::substr(errPath, 0, qt0 + 1) + path + pragma::string::substr(errPath, qt1);
	}
}

bool Lua::get_callstack(lua::State *l, std::stringstream &ss)
{
	int32_t level = 1;
	lua::DebugInfo d {};
	auto r = lua::get_stack(l, level, &d);
	if(r == 0)
		return false;
	while(r == 1) {
		if(lua::get_debug_info(l, "Sln", &d) != 0) {
			std::string t(level * 4, ' ');
			if(level >= 10) {
				ss << "\n" << t << "...";
				break;
			}
			else {
				auto filename = pragma::scripting::lua_core::util::make_clickable_lua_script_link(get_source(d), d.currentline);
				ss << "\n" << t << level << ": " << (d.name != nullptr ? d.name : "?") << "[" << d.linedefined << ":" << d.lastlinedefined << "] [" << d.what << ":" << d.namewhat << "] : " << filename;
			}
		}
		++level;
		r = lua::get_stack(l, level, &d);
	}
	return true;
}

bool Lua::PrintTraceback(lua::State *l, std::stringstream &ssOut, const std::string *pOptErrMsg, std::string *optOutFormattedErrMsg)
{
	lua::DebugInfo d {};
	int32_t level = 1;
	auto bFoundSrc = false;
	auto bNl = false;
	while(bFoundSrc == false && lua::get_stack(l, level, &d) == 1) {
		if(lua::get_debug_info(l, "Sln", &d) != 0 && (strcmp(d.what, "Lua") == 0 || strcmp(d.what, "main") == 0)) {
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
			auto c = pragma::fs::get_directory_separator();
			auto br = shortSrc.find(c);
			uint32_t offset = 0;
			auto bFound = false;
			auto luaPath = SCRIPT_DIRECTORY + c;
			while(br != std::string::npos && shortSrc.length() >= offset + luaPath.length() && (bFound = pragma::string::compare(shortSrc.data() + offset, luaPath.c_str(), false, luaPath.length())) == false) {
				offset = br + 1;
				br = shortSrc.find(c, br + 1);
			}
			if(bFound == true) {
				shortSrc = pragma::string::substr(shortSrc, offset);
				if(shortSrc.length() > maxLuaPathLen)
					shortSrc = "..." + shortSrc.substr(shortSrc.size() - maxLuaPathLen);
				//shortSrc = "[string \"" + shortSrc + "\"]";
				//open_lua_file(fname,lineId);
			}
			std::stringstream ssErrMsg;
			auto lineMsg = pragma::scripting::lua_core::util::make_clickable_lua_script_link(shortSrc, d.currentline);
			ssErrMsg << lineMsg << " " << errMsg;
			errMsg = ssErrMsg.str();
		}
		transform_path(d, errMsg, d.currentline);
		ssOut << errMsg;
		bNl = pragma::scripting::lua_core::util::get_code_snippet(ssOut, get_source(d), d.currentline, ":");
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

void Lua::PrintTraceback(lua::State *l, const std::string *pOptErrMsg)
{
	std::stringstream ssTbMsg;
	Lua::PrintTraceback(l, ssTbMsg, pOptErrMsg);
	auto tbMsg = ssTbMsg.str();

	std::stringstream ss;
	pragma::scripting::lua_core::util::get_lua_doc_info(ss, tbMsg);
	Con::COUT << ss.str();
	Con::flush();
}

int Lua::HandleTracebackError(lua::State *l)
{
	if(!IsString(l, -1))
		return 1;
	std::string msg = ToString(l, -1);
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
	auto errInfo = pragma::scripting::lua_core::util::parse_syntax_error_message(msg);
	if(!errInfo)
		return msg;
	std::stringstream ssMsg;
	pragma::scripting::lua_core::util::get_code_snippet(ssMsg, optFilename ? *optFilename : errInfo->first, errInfo->second, ":");
	return ssMsg.str();
}

static void handle_syntax_error(lua::State *l, Lua::StatusCode r, const std::string *fileName)
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

void Lua::HandleSyntaxError(lua::State *l, StatusCode r, const std::string &fileName) { handle_syntax_error(l, r, &fileName); }

void Lua::HandleSyntaxError(lua::State *l, StatusCode r) { handle_syntax_error(l, r, nullptr); }

void Lua::initialize_error_handler()
{
	luabind::register_exception_handler<Exception>(+[](lua::State *L, const Exception &e) { PushString(L, e.what()); });
	luabind::set_pcall_callback([](lua::State *l) -> void {
		PushCFunction(l, [](lua::State *l) -> int32_t {
			if(IsString(l, -1) == false)
				return 0; // This should never happen
			std::string errMsg = CheckString(l, -1);
			auto formattedMsg = pragma::scripting::lua_core::format_error_message(l, errMsg, StatusCode::ErrorRun, nullptr);
			pragma::scripting::lua_core::submit_error(l, formattedMsg);
			return 0;
		});
	});
}
