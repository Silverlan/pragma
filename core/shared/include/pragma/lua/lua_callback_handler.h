/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_CALLBACK_HANDLER_H__
#define __LUA_CALLBACK_HANDLER_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luacallback.h"
#include <sharedutils/scope_guard.h>
#include <sharedutils/util_event_reply.hpp>

class DLLNETWORK LuaCallbackHandler {
  public:
	// Lua Callbacks
	CallbackHandle AddLuaCallback(std::string identifier, const luabind::object &o);
	std::vector<CallbackHandle> *GetLuaCallbacks(std::string identifier);
	void CallLuaCallbacks(const std::string &name);
	template<class T, typename... TARGS>
	T CallLuaCallbacks(std::string name, TARGS... args);
	template<class T, typename... TARGS>
	CallbackReturnType CallLuaCallbacks(std::string name, T *ret, TARGS... args);
	template<typename... TARGS>
	util::EventReply CallLuaEvents(std::string name, TARGS... args);
  protected:
	std::unordered_map<std::string, std::vector<CallbackHandle>> m_luaCallbacks;
  private:
	std::queue<std::pair<std::string, CallbackHandle>> m_addQueue = {};
	uint32_t m_callDepth = 0u;
};

template<class T, typename... TARGS>
T LuaCallbackHandler::CallLuaCallbacks(std::string name, TARGS... args)
{
	++m_callDepth;
	util::ScopeGuard sg([this]() {
		if(--m_callDepth == 0u) {
			while(m_addQueue.empty() == false) {
				auto &pair = m_addQueue.front();
				auto it = m_luaCallbacks.find(pair.first);
				if(it == m_luaCallbacks.end())
					it = m_luaCallbacks.insert(std::unordered_map<std::string, std::vector<CallbackHandle>>::value_type(pair.first, std::vector<CallbackHandle>())).first;
				it->second.push_back(pair.second);

				m_addQueue.pop();
			}
		}
	});
	ustring::to_lower(name);
	auto it = m_luaCallbacks.find(name);
	if(it == m_luaCallbacks.end())
		return T();
	auto &callbacks = it->second;
	for(auto it = callbacks.begin(); it != callbacks.end();) {
		auto &hCallback = *it;
		if(hCallback.IsValid()) {
			auto *f = static_cast<LuaCallback *>(hCallback.get());
			if(std::is_same<T, void>::value == true)
				f->Call<T, TARGS...>(args...);
			else
				return f->Call<T, TARGS...>(args...);
			++it;
		}
		else
			it = callbacks.erase(it);
	}
	return T();
}
template<class T, typename... TARGS>
CallbackReturnType LuaCallbackHandler::CallLuaCallbacks(std::string name, T *ret, TARGS... args)
{
	++m_callDepth;
	util::ScopeGuard sg([this]() {
		if(--m_callDepth == 0u) {
			while(m_addQueue.empty() == false) {
				auto &pair = m_addQueue.front();
				auto it = m_luaCallbacks.find(pair.first);
				if(it == m_luaCallbacks.end())
					it = m_luaCallbacks.insert(std::unordered_map<std::string, std::vector<CallbackHandle>>::value_type(pair.first, std::vector<CallbackHandle>())).first;
				it->second.push_back(pair.second);

				m_addQueue.pop();
			}
		}
	});
	ustring::to_lower(name);
	auto it = m_luaCallbacks.find(name);
	if(it == m_luaCallbacks.end())
		return CallbackReturnType::NoReturnValue;
	auto &callbacks = it->second;
	for(auto it = callbacks.begin(); it != callbacks.end();) {
		auto &hCallback = *it;
		if(hCallback.IsValid()) {
			auto *f = static_cast<LuaCallback *>(hCallback.get());
			if(f->Call<T, TARGS...>(ret, args...) == true)
				return CallbackReturnType::HasReturnValue;
			++it;
		}
		else
			it = callbacks.erase(it);
	}
	return CallbackReturnType::NoReturnValue;
}

template<typename... TARGS>
util::EventReply LuaCallbackHandler::CallLuaEvents(std::string name, TARGS... args)
{
	++m_callDepth;
	util::ScopeGuard sg([this]() {
		if(--m_callDepth == 0u) {
			while(m_addQueue.empty() == false) {
				auto &pair = m_addQueue.front();
				auto it = m_luaCallbacks.find(pair.first);
				if(it == m_luaCallbacks.end())
					it = m_luaCallbacks.insert(std::unordered_map<std::string, std::vector<CallbackHandle>>::value_type(pair.first, std::vector<CallbackHandle>())).first;
				it->second.push_back(pair.second);

				m_addQueue.pop();
			}
		}
	});
	ustring::to_lower(name);
	auto it = m_luaCallbacks.find(name);
	if(it == m_luaCallbacks.end())
		return util::EventReply::Unhandled;
	auto &callbacks = it->second;
	for(auto it = callbacks.begin(); it != callbacks.end();) {
		auto &hCallback = *it;
		if(hCallback.IsValid()) {
			auto *f = static_cast<LuaCallback *>(hCallback.get());
			uint32_t reply = umath::to_integral(util::EventReply::Unhandled);
			if(f->Call<uint32_t, TARGS...>(&reply, args...) == true && static_cast<util::EventReply>(reply) == util::EventReply::Handled)
				return util::EventReply::Handled;
			++it;
		}
		else
			it = callbacks.erase(it);
	}
	return util::EventReply::Unhandled;
}

#endif
