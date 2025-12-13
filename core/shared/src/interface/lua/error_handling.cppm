// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.error_handling;

export import pragma.lua;

export namespace pragma::scripting::lua_core {
	enum class ErrorType : uint8_t { LoadError = 0, RuntimeError };
	// Note: In the case of a load error, optFilename will be assumed to be the file that failed to load
	DLLNETWORK std::string format_error_message(lua::State *l, const std::string &msg, Lua::StatusCode statusCode, const std::string *optFilename = nullptr, ErrorType errType = ErrorType::RuntimeError);
	// Calls lua_error
	DLLNETWORK void raise_error(lua::State *l, const std::string &msg);
	// Expects the error message to be on top of the stack
	DLLNETWORK void raise_error(lua::State *l);
	// Runs OnLuaError listeners and prints the error message
	DLLNETWORK void submit_error(lua::State *l, const std::string &msg);

	namespace util {
		DLLNETWORK std::string make_clickable_lua_script_link(const std::string &fileName, int32_t lineIdx);
		DLLNETWORK bool get_code_snippet(std::stringstream &outMsg, const std::string &fileName, uint32_t lineId, const std::string &prefix = "");
		DLLNETWORK void get_lua_doc_info(std::stringstream &outMsg, const std::string &errMsg);
		DLLNETWORK std::optional<std::pair<std::string, int32_t>> parse_syntax_error_message(const std::string &msg, size_t *optOutStartMsgPos = nullptr);
	};
};

export namespace Lua {
	DLLNETWORK void initialize_error_handler();
	DLLNETWORK bool get_callstack(lua::State *l, std::stringstream &ss);
	DLLNETWORK int HandleTracebackError(lua::State *l);
	DLLNETWORK bool PrintTraceback(lua::State *l, std::stringstream &ssOut, const std::string *pOptErrMsg = nullptr, std::string *optOutFormattedErrMsg = nullptr);
	DLLNETWORK void PrintTraceback(lua::State *l, const std::string *pOptErrMsg = nullptr);
	// Note: This function will attempt to retrieve the file name from the error message.
	// If the file name is truncated, this will not work! To be sure, define the third parameter as the actual file name.
	DLLNETWORK void HandleSyntaxError(lua::State *l, StatusCode r);
	DLLNETWORK void HandleSyntaxError(lua::State *l, StatusCode r, const std::string &fileName);

	DLLNETWORK void OpenFileInZeroBrane(const std::string &fname, uint32_t lineId);
	DLLNETWORK std::optional<std::string> GetLuaFilePath(const std::string &fname);
};
