// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.object_base;

import :scripting.lua.core;

export {
	class DLLNETWORK LuaObjectBase {
	  protected:
		std::shared_ptr<luabind::object> m_baseLuaObj;
		std::shared_ptr<luabind::weak_ref> m_weakRef;
	  public:
		LuaObjectBase();
		LuaObjectBase(const luabind::object &o);
		LuaObjectBase(const luabind::weak_ref &ref);
		void ClearLuaObject();
		void SetLuaObject(const luabind::object &o);
		void SetLuaObject(const luabind::weak_ref &ref);
		virtual ~LuaObjectBase() = default;
		luabind::object GetLuaObject() const;
		void CallLuaMember(const std::string &name);
		template<class T, typename... TARGS>
		T CallLuaMember(const std::string &name, TARGS... args)
		{
			auto o = GetLuaObject();

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
		CallbackReturnType CallLuaMember(const std::string &name, T *ret, TARGS... args)
		{
			auto o = GetLuaObject();

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
		std::string ToString() const;
	};
};
