// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.scripting.lua.util;

import pragma.scripting.lua.error_handling;

Lua::StatusCode pragma::scripting::lua::protected_call(lua_State *l, const std::function<Lua::StatusCode(lua_State *)> &pushFuncArgs, int32_t numResults)
{
    auto statusCode = Lua::ProtectedCall(l, pushFuncArgs, numResults, +[](lua_State *l) -> int32_t {
        if (!Lua::IsString(l, -1))
            return 0;
        auto *errMsg = Lua::CheckString(l, -1);
        auto newErrMsg = format_error_message(l, errMsg, Lua::StatusCode::ErrorRun, nullptr, ErrorType::RuntimeError);
        Lua::PushString(l, newErrMsg);
        Lua::StackDump(l);
        return 1;
    });
    if (statusCode == Lua::StatusCode::Ok)
        return statusCode;
    std::string errMsg;
    if (Lua::IsString(l, -1))
        errMsg = Lua::CheckString(l, -1);
    else
        errMsg = "Unknown Error!";
    submit_error(l, errMsg);
    return statusCode;
}
