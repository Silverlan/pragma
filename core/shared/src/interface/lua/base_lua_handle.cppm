// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.base_lua_handle;

import :scripting.lua.api;
import :scripting.lua.core;

export {
	namespace pragma {
		class DLLNETWORK BaseLuaHandle {
		  public:
			BaseLuaHandle();
			virtual ~BaseLuaHandle();
			util::TWeakSharedHandle<BaseLuaHandle> GetHandle() const { return pragma::util::TWeakSharedHandle<BaseLuaHandle> {m_handle}; }
			virtual void InitializeLuaObject(lua::State *lua) = 0;
			const luabind::object &GetLuaObject(lua::State *lua) const { return const_cast<BaseLuaHandle *>(this)->GetLuaObject(lua); }
			luabind::object &GetLuaObject(lua::State *lua) { return GetLuaObject(); }
			const luabind::object &GetLuaObject() const { return const_cast<BaseLuaHandle *>(this)->GetLuaObject(); }
			luabind::object &GetLuaObject() { return m_luaObj; }
			lua::State *GetLuaState() const;
			void PushLuaObject();
			void PushLuaObject(lua::State *l);

			void CallLuaMethod(const std::string &name);
			template<class T, typename... TARGS>
			T CallLuaMethod(const std::string &name, TARGS... args);
			template<class T, typename... TARGS>
			CallbackReturnType CallLuaMethod(const std::string &name, T *ret, TARGS... args);

			template<typename T>
			util::TWeakSharedHandle<T> GetHandle() const;
		  protected:
			template<typename T>
			void InitializeLuaObject(lua::State *l);
			void InvalidateHandle();
			void SetLuaObject(const luabind::object &o);
		  private:
			util::TSharedHandle<BaseLuaHandle> m_handle {};
			luabind::object m_luaObj {};
		};

		template<typename T>
		void BaseLuaHandle::InitializeLuaObject(lua::State *l)
		{
			m_luaObj = {l, LuaCore::raw_object_to_luabind_object(l, GetHandle<T>())};
		}

		template<typename T>
		util::TWeakSharedHandle<T> BaseLuaHandle::GetHandle() const
		{
			return util::weak_shared_handle_cast<BaseLuaHandle, T>(GetHandle());
		}

		template<class T, typename... TARGS>
		T BaseLuaHandle::CallLuaMethod(const std::string &name, TARGS... args)
		{
			auto &o = GetLuaObject();

			auto r = o[name];
			if(r) {
#ifndef LUABIND_NO_EXCEPTIONS
				try {
#endif
					return static_cast<T>(luabind::call_member<T>(o, name.c_str(), std::forward<TARGS>(args)...));
#ifndef LUABIND_NO_EXCEPTIONS
				}
				catch(luabind::error &err) {
					Lua::HandleLuaError(o.interpreter());
				}
				catch(const luabind::cast_failed &) // No return value was specified, or return value couldn't be cast
				{
					return T();
				}
#endif
			}
			return T();
		}
		template<class T, typename... TARGS>
		CallbackReturnType BaseLuaHandle::CallLuaMethod(const std::string &name, T *ret, TARGS... args)
		{
			auto &o = GetLuaObject();

			auto r = o[name];
			if(r) {
#ifndef LUABIND_NO_EXCEPTIONS
				try {
#endif
					*ret = static_cast<T>(luabind::call_member<T>(o, name.c_str(), std::forward<TARGS>(args)...));
#ifndef LUABIND_NO_EXCEPTIONS
				}
				catch(luabind::error &) {
					Lua::HandleLuaError(o.interpreter());
					return CallbackReturnType::NoReturnValue;
				}
				catch(const luabind::cast_failed &) // No return value was specified, or return value couldn't be cast
				{
					return CallbackReturnType::NoReturnValue;
				}
				catch(std::exception &) {
					return CallbackReturnType::NoReturnValue;
				}
#endif
				auto *state = r.interpreter();
				r.push(state);
				auto r = (Lua::IsCFunction(state, -1) == 0) ? CallbackReturnType::HasReturnValue : CallbackReturnType::NoReturnValue;
				Lua::Pop(state, 1);
				return r;
			}
			return CallbackReturnType::NoReturnValue;
		}
	}
};
