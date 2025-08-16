// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include <string>
#include <luasystem.h>

export module pragma.scripting.lua:error_handling;

export namespace pragma::scripting::lua {
    enum class ErrorType : uint8_t {
        LoadError = 0,
        RuntimeError
    };
    // Note: In the case of a load error, optFilename will be assumed to be the file that failed to load
    DLLNETWORK std::string format_error_message(lua_State *l, const std::string &msg, Lua::StatusCode statusCode, const std::string *optFilename = nullptr, ErrorType errType = ErrorType::RuntimeError);
    // Calls lua_error
    DLLNETWORK void raise_error(lua_State *l, const std::string &msg);
    // Expects the error message to be on top of the stack
    DLLNETWORK void raise_error(lua_State *l);
    // Runs OnLuaError listeners and prints the error message
    DLLNETWORK void submit_error(lua_State *l, const std::string &msg);

    namespace util {
        DLLNETWORK std::string make_clickable_lua_script_link(const std::string &fileName, int32_t lineIdx);
        DLLNETWORK bool get_code_snippet(std::stringstream &outMsg, const std::string &fileName, uint32_t lineId, const std::string &prefix = "");
        DLLNETWORK void get_lua_doc_info(std::stringstream &outMsg, const std::string &errMsg);
        DLLNETWORK std::optional<std::pair<std::string, int32_t>> parse_syntax_error_message(const std::string &msg, size_t *optOutStartMsgPos = nullptr);
    };
};
