// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.util;

static int32_t traceback(lua::State *l)
{
	if(!Lua::IsString(l, -1))
		return 0;
	auto *errMsg = Lua::CheckString(l, -1);
	auto newErrMsg = pragma::scripting::lua_core::format_error_message(l, errMsg, Lua::StatusCode::ErrorRun, nullptr, pragma::scripting::lua_core::ErrorType::RuntimeError);
	Lua::PushString(l, newErrMsg);
	return 1;
}

luabind::detail::function_object *pragma::scripting::lua_core::util::get_function_object(luabind::object const &fn)
{
	lua::State *L = fn.interpreter();
	{
		fn.push(L);
		luabind::detail::stack_pop pop(L, 1);
		if(!luabind::detail::is_luabind_function(L, -1)) {
			return nullptr;
		}
	}
	return *luabind::touserdata<luabind::detail::function_object *>(std::get<1>(luabind::getupvalue(fn, 1)));
}

Lua::StatusCode pragma::scripting::lua_core::protected_call(lua::State *l, const std::function<Lua::StatusCode(lua::State *)> &pushFuncArgs, int32_t numResults, std::string *optOutErrMsg)
{
	std::string errMsg;
	auto statusCode = Lua::ProtectedCall(l, pushFuncArgs, numResults, errMsg, &traceback);
	if(statusCode == Lua::StatusCode::Ok)
		return statusCode;
	if(optOutErrMsg)
		*optOutErrMsg = std::move(errMsg);
	else
		submit_error(l, errMsg);
	return statusCode;
}

Lua::StatusCode pragma::scripting::lua_core::protected_call(lua::State *l, int32_t numArgs, int32_t numResults, std::string *optOutErrMsg)
{
	std::string errMsg;
	auto statusCode = Lua::ProtectedCall(l, numArgs, numResults, errMsg, &traceback);
	if(statusCode == Lua::StatusCode::Ok)
		return statusCode;
	if(optOutErrMsg)
		*optOutErrMsg = std::move(errMsg);
	else
		submit_error(l, errMsg);
	return statusCode;
}

Lua::StatusCode pragma::scripting::lua_core::run_string(lua::State *l, const std::string &str, const std::string &chunkName, int32_t numResults, std::string *optOutErrMsg)
{
	std::string errMsg;
	auto statusCode = Lua::RunString(l, str, numResults, chunkName, errMsg, &traceback, [](lua::State *l, Lua::StatusCode statusCode) {
		if(!Lua::IsString(l, -1))
			return;
		auto *errorMessage = Lua::CheckString(l, -1);
		auto formattedMsg = format_error_message(l, errorMessage, statusCode, nullptr, ErrorType::LoadError);
		Lua::Pop(l, 1); // Pop the original error message
		Lua::PushString(l, formattedMsg);
	});
	if(statusCode == Lua::StatusCode::Ok)
		return statusCode;
	if(optOutErrMsg)
		*optOutErrMsg = std::move(errMsg);
	else
		submit_error(l, errMsg);
	return statusCode;
}
