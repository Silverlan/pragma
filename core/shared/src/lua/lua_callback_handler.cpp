/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/lua_callback_handler.h"
#include "pragma/lua/luacallback.h"
#include "pragma/lua/luafunction_call.h"

CallbackHandle LuaCallbackHandler::AddLuaCallback(std::string identifier, const luabind::object &o)
{
	ustring::to_lower(identifier);
	if(m_callDepth > 0u) {
		// m_luaCallbacks is currently being iterated on, so we have to delay adding the new callback
		auto hCallback = CallbackHandle {std::shared_ptr<TCallback>(new LuaCallback(o))};
		m_addQueue.push(std::make_pair(identifier, hCallback));
		return hCallback;
	}
	auto it = m_luaCallbacks.find(identifier);
	if(it == m_luaCallbacks.end())
		it = m_luaCallbacks.insert(std::unordered_map<std::string, std::vector<CallbackHandle>>::value_type(identifier, std::vector<CallbackHandle>())).first;
	it->second.push_back(CallbackHandle {std::shared_ptr<TCallback>(new LuaCallback(o))});
	return it->second.back();
}

std::vector<CallbackHandle> *LuaCallbackHandler::GetLuaCallbacks(std::string identifier)
{
	ustring::to_lower(identifier);
	auto it = m_luaCallbacks.find(identifier);
	if(it == m_luaCallbacks.end())
		return nullptr;
	return &it->second;
}

void LuaCallbackHandler::CallLuaCallbacks(const std::string &name) { CallLuaCallbacks<void>(name); }
