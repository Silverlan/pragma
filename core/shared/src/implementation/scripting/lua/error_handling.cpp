// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.error_handling;

static spdlog::logger &LOGGER = pragma::register_logger("lua");

std::optional<std::pair<std::string, int32_t>> pragma::scripting::lua_core::util::parse_syntax_error_message(const std::string &msg, size_t *optOutStartMsgPos)
{
	if (optOutStartMsgPos)
		*optOutStartMsgPos = std::string::npos;
	auto &err = msg;
	auto brSt = err.find('[');
	auto brEn = err.find(']', brSt + 1);
	if(brSt == std::string::npos || brEn == std::string::npos) {
		auto c0 = err.find(':');
		if(c0 == std::string::npos)
			return {};
		auto c1 = err.find(':', c0 + 1);
		if(c1 == std::string::npos)
			return {};
		auto filename = err.substr(0, c0);
		auto lineId = pragma::util::to_int(err.substr(c0 + 1, (c1 - c0) - 1));
		if(optOutStartMsgPos)
			*optOutStartMsgPos = err.find_first_not_of(string::WHITESPACE, c1);
		return std::pair<std::string, int32_t> {filename, lineId};
	}

	auto qSt = err.find('\"', brSt);
	auto qEn = err.find('\"', qSt + 1);
	if(qSt == std::string::npos || qEn == std::string::npos)
		return {};
	std::string filename = err.substr(qSt + 1, qEn - qSt - 1);
	auto cSt = err.find(':', brEn + 1);
	auto cEn = err.find(':', cSt + 1);
	if(cSt == std::string::npos || cEn == std::string::npos)
		return {};
	if(optOutStartMsgPos)
		*optOutStartMsgPos = err.find_first_not_of(string::WHITESPACE, cEn);
	auto lineId = pragma::util::to_int(err.substr(cSt + 1, cEn - cSt - 1));
	return std::pair<std::string, int32_t> {filename, lineId};
}

std::string pragma::scripting::lua_core::util::make_clickable_lua_script_link(const std::string &fileName, int32_t lineIdx)
{
	auto absPath = fileName;
	fs::find_absolute_path(absPath, absPath);
	return pragma::util::make_clickable_link(absPath, lineIdx);
}

bool pragma::scripting::lua_core::util::get_code_snippet(std::stringstream &outMsg, const std::string &fileName, uint32_t lineId, const std::string &prefix)
{
	auto fname = fileName;

	std::string ext;
	if(ufile::get_extension(fname, &ext) == true && ext == Lua::FILE_EXTENSION_PRECOMPILED) {
		//ssMsg<<"[PRECOMPILED - NO DEBUG INFORMATION AVAILABLE]";
		return false;
	}
	fname = fs::get_normalized_path(fname);
	auto br = fname.find_first_of(fs::get_directory_separator());
	while(br != std::string::npos && fname.substr(0, br) != Lua::SCRIPT_DIRECTORY) {
		fname = fname.substr(br + 1);
		br = fname.find_first_of(fs::get_directory_separator());
	}
	auto f = fs::open_file(fname, fs::FileMode::Read | fs::FileMode::Binary);
	if(f != nullptr) {
		char c = 0;
		uint32_t curLineId = 1;
		auto linePrint = static_cast<uint32_t>(math::max(static_cast<int32_t>(lineId) - 2, static_cast<int32_t>(1)));
		auto lastLinePrint = lineId + 2;
		while(!f->Eof() && curLineId < linePrint) {
			f->Read(&c, 1);
			if(c == '\n')
				++curLineId;
		}
		if(!f->Eof() && curLineId == linePrint) {
			outMsg << prefix;
			for(auto i = linePrint; i <= lastLinePrint; ++i) {
				auto l = f->ReadLine();
				if(i == lineId)
					outMsg << "\n  > ";
				else
					outMsg << "\n    ";
				outMsg << l;
				if(f->Eof())
					break;
			}
			return true;
		}
	}
	return false;
}

void pragma::scripting::lua_core::util::get_lua_doc_info(std::stringstream &outMsg, const std::string &errMsg)
{
	outMsg << errMsg << "\n";

	std::string cause {};
	const std::string errMsgIdentifier = "No matching overload found";
	auto posErrOverload = errMsg.find(errMsgIdentifier);
	if(posErrOverload != std::string::npos) {
		// Luabind error, try to retrieve function name
		auto posEnd = errMsg.find('(', posErrOverload + errMsgIdentifier.length());
		auto pos = errMsg.rfind(' ', posEnd);
		if(pos != std::string::npos && posEnd != std::string::npos)
			cause = string::substr(errMsg, pos + 1, posEnd - pos - 1);
	}
	else {
		auto pos = errMsg.find('\'');
		auto posEnd = errMsg.find('\'', pos + 1);
		if(posEnd != std::string::npos)
			cause = string::substr(errMsg, pos + 1, posEnd - pos - 1);
	}
	if(cause.empty() == false)
		Lua::doc::print_documentation(cause, outMsg);

	outMsg << pragma::console::get_ansi_color_code(console::ConsoleColorFlags::Reset);
	outMsg << "You can use the console command 'lua_help <name>' to get more information about a specific function/library/etc.\n";
	outMsg << "\n";
}

std::string pragma::scripting::lua_core::format_error_message(lua::State *l, const std::string &msg, Lua::StatusCode statusCode, const std::string *optFilename, ErrorType errType)
{
	const char *prefixMarker = "error info: "; // A marker to indicate whether the message has already been formatted
	if(msg.find(prefixMarker) != std::string::npos) {
		// The error message has already been formatted
		// and there is nothing for us to do.
		return msg;
	}

	auto formattedMsg = msg;
	if(errType == ErrorType::LoadError) {
		// We can't get additional debug information if an error occurred during loading,
		// and the filename will likely be truncated, so we'll generate our own message.
		switch(statusCode) {
		case Lua::StatusCode::ErrorFile:
		case Lua::StatusCode::ErrorSyntax:
		case Lua::StatusCode::ErrorMemory:
			{
				size_t startMsgPos;
				std::stringstream ssMsg;
				ssMsg << magic_enum::enum_name(errType) << ":" << magic_enum::enum_name(statusCode) << " ";

				auto errInfo = util::parse_syntax_error_message(formattedMsg, &startMsgPos);
				if(errInfo) {
					std::string filename;
					if(optFilename)
						filename = *optFilename;
					else
						filename = errInfo->first;
					auto formattedFilename = util::make_clickable_lua_script_link(filename, errInfo->second);
					ssMsg << formattedFilename;
					if (startMsgPos != std::string::npos)
						ssMsg << " " << formattedMsg.substr(startMsgPos);
					else
						ssMsg << " " << formattedMsg;
					auto bNl = util::get_code_snippet(ssMsg, pragma::util::FilePath(Lua::SCRIPT_DIRECTORY, filename).GetString(), errInfo->second, ":");
					if(bNl == true)
						ssMsg << "\n\n";
					else
						ssMsg << ":\n";
				}
				else
					ssMsg << formattedMsg << "\n";
				ssMsg << "    Callstack:";
				Lua::get_callstack(l, ssMsg);
				formattedMsg = ssMsg.str();
				break;
			}
		}
	}
	else {
		std::stringstream ss;
		Lua::PrintTraceback(l, ss, &formattedMsg);
		formattedMsg = ss.str();
	}

	std::stringstream ss;
	util::get_lua_doc_info(ss, formattedMsg);
	return prefixMarker +ss.str();
}

void pragma::scripting::lua_core::raise_error(lua::State *l, const std::string &msg) { Lua::Error(l, msg); }
void pragma::scripting::lua_core::raise_error(lua::State *l) { Lua::Error(l); }

void pragma::scripting::lua_core::submit_error(lua::State *l, const std::string &msg)
{
	auto *nw = get_engine()->GetNetworkState(l);
	auto *game = nw ? nw->GetGameState() : nullptr;
	if(game)
		game->CallLuaCallbacks<void, std::string>("OnLuaError", msg);
	LOGGER.error(msg);
}
