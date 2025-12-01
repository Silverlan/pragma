// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.function;

import :scripting.lua.core;

export {
	//namespace luabind{class object;}
	class DLLNETWORK LuaFunction {
	  protected:
		std::shared_ptr<luabind::object> m_luaFunction;
	  public:
		LuaFunction(const luabind::object &o);
		LuaFunction(const LuaFunction &other);
		LuaFunction(std::nullptr_t);
		bool operator==(std::nullptr_t);
		virtual ~LuaFunction();
		luabind::object &GetLuaObject();
		void operator()();
		template<class T, typename... TARGS>
		T Call(TARGS... args)
		{
			auto &r = m_luaFunction;
#ifndef LUABIND_NO_EXCEPTIONS
			try {
#endif
				return static_cast<T>(luabind::call_function<T>(*r, std::forward<TARGS>(args)...));
#ifndef LUABIND_NO_EXCEPTIONS
			}
			catch(const luabind::error &) {
				Lua::HandleLuaError(r->interpreter());
			}
			catch(const luabind::cast_failed &) {
				return T();
			}
#endif
			return T();
		}
		template<class T, typename... TARGS>
		bool Call(T *ret, TARGS... args)
		{
			auto &r = m_luaFunction;
#ifndef LUABIND_NO_EXCEPTIONS
			try {
#endif
				*ret = static_cast<T>(luabind::call_function<T>(*r, std::forward<TARGS>(args)...));
#ifndef LUABIND_NO_EXCEPTIONS
			}
			catch(luabind::error &) {
				Lua::HandleLuaError(r->interpreter());
				return false;
			}
			catch(std::exception &) {
				return false;
			}
#endif

			// TODO: What was this for?
			/*auto *state = r->interpreter();
			r->push(state);
			auto cret = (Lua::IsCFunction(state, -1) == 0) ? true : false;
			Lua::Pop(state, 1);
			return cret;*/

			return true;
		}
	};
}
