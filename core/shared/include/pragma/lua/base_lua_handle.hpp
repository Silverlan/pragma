/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_BASE_LUA_HANDLE_HPP__
#define __PRAGMA_BASE_LUA_HANDLE_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <sharedutils/util_shared_handle.hpp>
#include <sharedutils/functioncallback.h>
#include "pragma/lua/lua_handles.hpp"
#include "pragma/lua/raw_object.hpp"

namespace pragma {
	class DLLNETWORK BaseLuaHandle {
	  public:
		BaseLuaHandle();
		virtual ~BaseLuaHandle();
		util::TWeakSharedHandle<BaseLuaHandle> GetHandle() const { return util::TWeakSharedHandle<BaseLuaHandle> {m_handle}; }
		virtual void InitializeLuaObject(lua_State *lua) = 0;
		const luabind::object &GetLuaObject() const { return const_cast<BaseLuaHandle *>(this)->GetLuaObject(); }
		luabind::object &GetLuaObject() { return m_luaObj; }
		lua_State *GetLuaState() const;
		void PushLuaObject();
		void PushLuaObject(lua_State *l);

		void CallLuaMethod(const std::string &name);
		template<class T, typename... TARGS>
		T CallLuaMethod(const std::string &name, TARGS... args);
		template<class T, typename... TARGS>
		CallbackReturnType CallLuaMethod(const std::string &name, T *ret, TARGS... args);

		template<typename T>
		util::TWeakSharedHandle<T> GetHandle() const;
	  protected:
		template<typename T>
		void InitializeLuaObject(lua_State *l);
		void InvalidateHandle();
		void SetLuaObject(const luabind::object &o);
	  private:
		util::TSharedHandle<BaseLuaHandle> m_handle {};
		luabind::object m_luaObj {};
	};
};

template<typename T>
void pragma::BaseLuaHandle::InitializeLuaObject(lua_State *l)
{
	m_luaObj = {l, pragma::lua::raw_object_to_luabind_object(l, GetHandle<T>())};
}

template<typename T>
util::TWeakSharedHandle<T> pragma::BaseLuaHandle::GetHandle() const
{
	return util::weak_shared_handle_cast<BaseLuaHandle, T>(GetHandle());
}

#endif
