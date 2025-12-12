// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.callback_handler;

import :scripting.lua.callback;

export {
	class DLLNETWORK LuaCallbackHandler {
	  public:
		// Lua Callbacks
		CallbackHandle AddLuaCallback(std::string identifier, const luabind::object &o);
		std::vector<CallbackHandle> *GetLuaCallbacks(std::string identifier);
		void CallLuaCallbacks(const std::string &name);
		template<class T, typename... TARGS>
		T CallLuaCallbacks(std::string name, TARGS... args)
		{
			++m_callDepth;
			pragma::util::ScopeGuard sg([this]() {
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
			pragma::string::to_lower(name);
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
		CallbackReturnType CallLuaCallbacks(std::string name, T *ret, TARGS... args)
		{
			++m_callDepth;
			pragma::util::ScopeGuard sg([this]() {
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
			pragma::string::to_lower(name);
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
		pragma::util::EventReply CallLuaEvents(std::string name, TARGS... args)
		{
			++m_callDepth;
			pragma::util::ScopeGuard sg([this]() {
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
			pragma::string::to_lower(name);
			auto it = m_luaCallbacks.find(name);
			if(it == m_luaCallbacks.end())
				return pragma::util::EventReply::Unhandled;
			auto &callbacks = it->second;
			for(auto it = callbacks.begin(); it != callbacks.end();) {
				auto &hCallback = *it;
				if(hCallback.IsValid()) {
					auto *f = static_cast<LuaCallback *>(hCallback.get());
					uint32_t reply = pragma::math::to_integral(pragma::util::EventReply::Unhandled);
					if(f->Call<uint32_t, TARGS...>(&reply, args...) == true && static_cast<pragma::util::EventReply>(reply) == pragma::util::EventReply::Handled)
						return pragma::util::EventReply::Handled;
					++it;
				}
				else
					it = callbacks.erase(it);
			}
			return pragma::util::EventReply::Unhandled;
		}
	  protected:
		std::unordered_map<std::string, std::vector<CallbackHandle>> m_luaCallbacks;
	  private:
		std::queue<std::pair<std::string, CallbackHandle>> m_addQueue = {};
		uint32_t m_callDepth = 0u;
	};
};
