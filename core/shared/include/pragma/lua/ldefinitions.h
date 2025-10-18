// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LDEFINITIONS_H__
#define __LDEFINITIONS_H__

#include "pragma/networkdefinitions.h"
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/pseudo_traits.hpp>
#include <luabind/make_function_signature.hpp>
#include "pragma/lua/ldefinitions.h"
#include <type_traits>

#define luaL_checkboolean(L, n) (luaL_checktype(L, (n), LUA_TBOOLEAN))
#define luaL_checkfunction(L, n) (luaL_checktype(L, (n), LUA_TFUNCTION))
#define luaL_checklightuserdata(L, n) (luaL_checktype(L, (n), LUA_TLIGHTUSERDATA))
#define luaL_checknil(L, n) (luaL_checktype(L, (n), LUA_TNIL))
#define luaL_checktable(L, n) (luaL_checktype(L, (n), LUA_TTABLE))
#define luaL_checkthread(L, n) (luaL_checktype(L, (n), LUA_TTHREAD))
#define luaL_checkuserdata(L, n) (luaL_checktype(L, (n), LUA_TUSERDATA))

inline int lua_createreference(lua_State *l, int index)
{
	lua_pushvalue(l, index);
	return luaL_ref(l, LUA_REGISTRYINDEX);
}

inline void lua_removereference(lua_State *l, int index) { luaL_unref(l, LUA_REGISTRYINDEX, index); }

#define lua_registerglobalint(global)                                                                                                                                                                                                                                                            \
    lua_pushinteger(m_lua, global);                                                                                                                                                                                                                                                              \
    lua_setglobal(m_lua, #global);

#define STR(arg) #arg
#define lua_registerglobalenum(en, name)                                                                                                                                                                                                                                                         \
    lua_pushinteger(m_lua, int(en::name));                                                                                                                                                                                                                                                       \
    lua_setglobal(m_lua, STR(en##_##name));

#define lua_checkentityret(l, hEnt, ret)                                                                                                                                                                                                                                                         \
    if(!hEnt->IsValid()) {                                                                                                                                                                                                                                                                       \
        lua_pushstring(l, "Attempted to use a NULL entity");                                                                                                                                                                                                                                     \
        lua_error(l);                                                                                                                                                                                                                                                                            \
        return ret;                                                                                                                                                                                                                                                                              \
    }

#define lua_checkentity(l, hEnt)                                                                                                                                                                                                                                                                 \
    if(!hEnt->IsValid()) {                                                                                                                                                                                                                                                                       \
        lua_pushstring(l, "Attempted to use a NULL entity");                                                                                                                                                                                                                                     \
        lua_error(l);                                                                                                                                                                                                                                                                            \
        return;                                                                                                                                                                                                                                                                                  \
    }

#define lua_checktimer(l, hTimer)                                                                                                                                                                                                                                                                \
    {                                                                                                                                                                                                                                                                                            \
        if(hTimer.IsValid() == false) {                                                                                                                                                                                                                                                          \
            lua_pushstring(l, "Attempted to use a NULL timer");                                                                                                                                                                                                                                  \
            lua_error(l);                                                                                                                                                                                                                                                                        \
            return;                                                                                                                                                                                                                                                                              \
        }                                                                                                                                                                                                                                                                                        \
    }

#define lua_pushentity(luastate, ent)                                                                                                                                                                                                                                                            \
    {                                                                                                                                                                                                                                                                                            \
        ent->GetLuaObject().push(luastate);                                                                                                                                                                                                                                                      \
    }

#define lua_pushtablecfunction(l, tablename, funcname, cfuncref)                                                                                                                                                                                                                                 \
    lua_getglobal(l, tablename);                                                                                                                                                                                                                                                                 \
    if(lua_istable(l, -1)) {                                                                                                                                                                                                                                                                     \
        int top = lua_gettop(l);                                                                                                                                                                                                                                                                 \
        lua_pushstring(l, funcname);                                                                                                                                                                                                                                                             \
        lua_pushcfunction(l, cfuncref);                                                                                                                                                                                                                                                          \
        lua_settable(l, top);                                                                                                                                                                                                                                                                    \
    }                                                                                                                                                                                                                                                                                            \
    lua_pop(l, 1);

#define lua_gettablefunction(lstate, tablename, funcname, oncall)                                                                                                                                                                                                                                \
    lua_getglobal(lstate, tablename);                                                                                                                                                                                                                                                            \
    if(lua_istable(lstate, -1)) {                                                                                                                                                                                                                                                                \
        lua_getfield(lstate, -1, funcname);                                                                                                                                                                                                                                                      \
        if(lua_isfunction(lstate, -1)) {                                                                                                                                                                                                                                                         \
            oncall                                                                                                                                                                                                                                                                               \
        }                                                                                                                                                                                                                                                                                        \
    }                                                                                                                                                                                                                                                                                            \
    lua_pop(l, 1);

#define lua_callfunction(nwstate, func)                                                                                                                                                                                                                                                          \
    {                                                                                                                                                                                                                                                                                            \
        lua_State *l = nwstate->GetLuaState();                                                                                                                                                                                                                                                   \
        lua_rawgeti(l, LUA_REGISTRYINDEX, func);                                                                                                                                                                                                                                                 \
        luaerror(nwstate, lua_pcall(l, 0, 0, 0));                                                                                                                                                                                                                                                \
    }

#define lua_calltablefunction(lstate, tablename, funcname, numreturn, oncall)                                                                                                                                                                                                                    \
    lua_getglobal(lstate, tablename);                                                                                                                                                                                                                                                            \
    if(lua_istable(lstate, -1)) {                                                                                                                                                                                                                                                                \
        lua_getfield(lstate, -1, funcname);                                                                                                                                                                                                                                                      \
        if(lua_isfunction(lstate, -1)) {                                                                                                                                                                                                                                                         \
            if(lua_pcall((lstate), (0), (numreturn), (0)) == 0) {                                                                                                                                                                                                                                \
                oncall                                                                                                                                                                                                                                                                           \
            }                                                                                                                                                                                                                                                                                    \
        }                                                                                                                                                                                                                                                                                        \
    }                                                                                                                                                                                                                                                                                            \
    lua_pop(l, 1);

#define COMMA ,
#define lua_bind(data) luabind::module(m_lua)[data];

inline const char *lua_gettype(lua_State *l, int n)
{
	const char *arg = lua_tostring(l, n);
	if(arg == NULL) {
		int type = lua_type(l, n);
		switch(type) {
		case LUA_TNIL:
			{
				arg = "nil";
				break;
			}
		case LUA_TBOOLEAN:
			{
				arg = "Boolean";
				break;
			}

		case LUA_TLIGHTUSERDATA:
			{
				arg = "LightUserData";
				break;
			}
		case LUA_TNUMBER:
			{
				arg = "Number";
				break;
			}
		case LUA_TSTRING:
			{
				arg = "String";
				break;
			}
		case LUA_TTABLE:
			{
				arg = "Table";
				break;
			}
		case LUA_TFUNCTION:
			{
				arg = "Function";
				break;
			}
		case LUA_TUSERDATA:
			{
				arg = "UserData";
				break;
			}
		case LUA_TTHREAD:
			{
				arg = "Thread";
				break;
			}
		default:
			arg = "Unknown";
		}
	}
	return arg;
};

#define lua_registercheck(type, cls)                                                                                                                                                                                                                                                             \
	inline cls *_lua_##type##_check(lua_State *l, int n)                                                                                                                                                                                                                                         \
	{                                                                                                                                                                                                                                                                                            \
		luaL_checkuserdata(l, n);                                                                                                                                                                                                                                                                \
		luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                            \
		auto *pValue = luabind::object_cast_nothrow<cls *>(o, static_cast<cls *>(nullptr));                                                                                                                                                                                                      \
		if(pValue == nullptr) {                                                                                                                                                                                                                                                                  \
			std::string err = #type " expected, got ";                                                                                                                                                                                                                                           \
			err += lua_gettype(l, n);                                                                                                                                                                                                                                                            \
			luaL_argerror(l, n, err.c_str());                                                                                                                                                                                                                                                    \
		}                                                                                                                                                                                                                                                                                        \
		return pValue;                                                                                                                                                                                                                                                                           \
	}                                                                                                                                                                                                                                                                                            \
	inline bool _lua_is##type(lua_State *l, int n)                                                                                                                                                                                                                                               \
	{                                                                                                                                                                                                                                                                                            \
		if(!lua_isuserdata(l, n))                                                                                                                                                                                                                                                                \
			return false;                                                                                                                                                                                                                                                                        \
		luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                            \
		auto *pValue = luabind::object_cast_nothrow<cls *>(o, static_cast<cls *>(nullptr));                                                                                                                                                                                                      \
		return (pValue != nullptr) ? true : false;                                                                                                                                                                                                                                               \
	}                                                                                                                                                                                                                                                                                            \
	inline cls *_lua_##type##_get(lua_State *l, int n)                                                                                                                                                                                                                                           \
	{                                                                                                                                                                                                                                                                                            \
		luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                            \
		return luabind::object_cast_nothrow<cls *>(o, static_cast<cls *>(nullptr));                                                                                                                                                                                                              \
	}                                                                                                                                                                                                                                                                                            \
	namespace Lua {                                                                                                                                                                                                                                                                              \
		static cls *(&Check##type)(lua_State *, int) = _lua_##type##_check;                                                                                                                                                                                                                      \
		static bool (&Is##type)(lua_State *, int) = _lua_is##type;                                                                                                                                                                                                                               \
		static cls *(&To##type)(lua_State *, int) = _lua_##type##_get;                                                                                                                                                                                                                           \
	};

#define lua_registercheck_shared_ptr(type, cls)                                                                                                                                                                                                                                                  \
	inline cls *_lua_##type##_check(lua_State *l, int n)                                                                                                                                                                                                                                         \
	{                                                                                                                                                                                                                                                                                            \
		luaL_checkuserdata(l, n);                                                                                                                                                                                                                                                                \
		luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                            \
		auto *pValue = luabind::object_cast_nothrow<std::shared_ptr<cls> *>(o, static_cast<std::shared_ptr<cls> *>(nullptr));                                                                                                                                                                    \
		if(pValue == nullptr || pValue->get() == NULL) {                                                                                                                                                                                                                                         \
			std::string err = #type " expected, got ";                                                                                                                                                                                                                                           \
			err += lua_gettype(l, n);                                                                                                                                                                                                                                                            \
			luaL_argerror(l, n, err.c_str());                                                                                                                                                                                                                                                    \
		}                                                                                                                                                                                                                                                                                        \
		return pValue->get();                                                                                                                                                                                                                                                                    \
	}                                                                                                                                                                                                                                                                                            \
	inline bool _lua_is##type(lua_State *l, int n)                                                                                                                                                                                                                                               \
	{                                                                                                                                                                                                                                                                                            \
		if(!lua_isuserdata(l, n))                                                                                                                                                                                                                                                                \
			return false;                                                                                                                                                                                                                                                                        \
		luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                            \
		auto *pValue = luabind::object_cast_nothrow<std::shared_ptr<cls> *>(o, static_cast<std::shared_ptr<cls> *>(nullptr));                                                                                                                                                                    \
		return (pValue != nullptr && pValue->get() != NULL) ? true : false;                                                                                                                                                                                                                      \
	}                                                                                                                                                                                                                                                                                            \
	inline cls *_lua_##type##_get(lua_State *l, int n)                                                                                                                                                                                                                                           \
	{                                                                                                                                                                                                                                                                                            \
		luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                            \
		auto *pValue = luabind::object_cast_nothrow<std::shared_ptr<cls> *>(o, static_cast<std::shared_ptr<cls> *>(nullptr));                                                                                                                                                                    \
		return pValue->get();                                                                                                                                                                                                                                                                    \
	}                                                                                                                                                                                                                                                                                            \
	namespace Lua {                                                                                                                                                                                                                                                                              \
		static cls *(&Check##type)(lua_State *, int) = _lua_##type##_check;                                                                                                                                                                                                                      \
		static bool (&Is##type)(lua_State *, int) = _lua_is##type;                                                                                                                                                                                                                               \
		static cls *(&To##type)(lua_State *, int) = _lua_##type##_get;                                                                                                                                                                                                                           \
	};

#define lua_registercheck_inherited_ptr(type, cls, pcls)                                                                                                                                                                                                                                         \
	inline cls *_lua_##type##_check(lua_State *l, int n)                                                                                                                                                                                                                                         \
	{                                                                                                                                                                                                                                                                                            \
		luaL_checkuserdata(l, n);                                                                                                                                                                                                                                                                \
		luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                            \
		auto *pValue = luabind::object_cast_nothrow<pcls *>(o, static_cast<pcls *>(nullptr));                                                                                                                                                                                                    \
		if(pValue == nullptr) {                                                                                                                                                                                                                                                                  \
			std::string err = #type " expected, got ";                                                                                                                                                                                                                                           \
			err += lua_gettype(l, n);                                                                                                                                                                                                                                                            \
			luaL_argerror(l, n, err.c_str());                                                                                                                                                                                                                                                    \
		}                                                                                                                                                                                                                                                                                        \
		return pValue;                                                                                                                                                                                                                                                                           \
	}                                                                                                                                                                                                                                                                                            \
	inline bool _lua_is##type(lua_State *l, int n)                                                                                                                                                                                                                                               \
	{                                                                                                                                                                                                                                                                                            \
		if(!lua_isuserdata(l, n))                                                                                                                                                                                                                                                                \
			return false;                                                                                                                                                                                                                                                                        \
		luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                            \
		auto *pValue = luabind::object_cast_nothrow<pcls *>(o, static_cast<pcls *>(nullptr));                                                                                                                                                                                                    \
		return (pValue != nullptr) ? true : false;                                                                                                                                                                                                                                               \
	}                                                                                                                                                                                                                                                                                            \
	inline cls *_lua_##type##_get(lua_State *l, int n)                                                                                                                                                                                                                                           \
	{                                                                                                                                                                                                                                                                                            \
		luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                            \
		auto *pValue = luabind::object_cast_nothrow<pcls *>(o, static_cast<pcls *>(nullptr));                                                                                                                                                                                                    \
		return pValue;                                                                                                                                                                                                                                                                           \
	}                                                                                                                                                                                                                                                                                            \
	namespace Lua {                                                                                                                                                                                                                                                                              \
		static cls *(&Check##type)(lua_State *, int) = _lua_##type##_check;                                                                                                                                                                                                                      \
		static bool (&Is##type)(lua_State *, int) = _lua_is##type;                                                                                                                                                                                                                               \
		static cls *(&To##type)(lua_State *, int) = _lua_##type##_get;                                                                                                                                                                                                                           \
	};

#define LUA_SETUP_HANDLE_CHECK(localname, classname, handlename)                                                                                                                                                                                                                                 \
	namespace Lua {                                                                                                                                                                                                                                                                              \
		static inline classname *Check##localname(lua_State *l, int n)                                                                                                                                                                                                                           \
		{                                                                                                                                                                                                                                                                                        \
			luaL_checkuserdata(l, n);                                                                                                                                                                                                                                                            \
			luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                        \
			auto *pValue = luabind::object_cast_nothrow<handlename *>(o, static_cast<handlename *>(nullptr));                                                                                                                                                                                    \
			if(pValue == nullptr) {                                                                                                                                                                                                                                                              \
				std::string err = #classname " expected, got ";                                                                                                                                                                                                                                  \
				err += lua_gettype(l, n);                                                                                                                                                                                                                                                        \
				luaL_argerror(l, n, err.c_str());                                                                                                                                                                                                                                                \
			}                                                                                                                                                                                                                                                                                    \
			if(!pValue->IsValid()) {                                                                                                                                                                                                                                                             \
				std::string err = "Attempted to use a NULL ";                                                                                                                                                                                                                                    \
				err += #localname;                                                                                                                                                                                                                                                               \
				std::transform(err.begin(), err.end(), err.begin(), ::tolower);                                                                                                                                                                                                                  \
				lua_pushstring(l, err.c_str());                                                                                                                                                                                                                                                  \
				lua_error(l);                                                                                                                                                                                                                                                                    \
			}                                                                                                                                                                                                                                                                                    \
			return pValue->get();                                                                                                                                                                                                                                                                \
		}                                                                                                                                                                                                                                                                                        \
		static inline handlename *Check##localname##Handle(lua_State *l, int n)                                                                                                                                                                                                                  \
		{                                                                                                                                                                                                                                                                                        \
			luaL_checkuserdata(l, n);                                                                                                                                                                                                                                                            \
			luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                        \
			auto *pValue = luabind::object_cast_nothrow<handlename *>(o, static_cast<handlename *>(nullptr));                                                                                                                                                                                    \
			if(pValue == nullptr) {                                                                                                                                                                                                                                                              \
				std::string err = #classname " expected, got ";                                                                                                                                                                                                                                  \
				err += lua_gettype(l, n);                                                                                                                                                                                                                                                        \
				luaL_argerror(l, n, err.c_str());                                                                                                                                                                                                                                                \
			}                                                                                                                                                                                                                                                                                    \
			return pValue;                                                                                                                                                                                                                                                                       \
		}                                                                                                                                                                                                                                                                                        \
		static inline bool Is##localname(lua_State *l, int n)                                                                                                                                                                                                                                    \
		{                                                                                                                                                                                                                                                                                        \
			if(!lua_isuserdata(l, n))                                                                                                                                                                                                                                                            \
				return false;                                                                                                                                                                                                                                                                    \
			luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                        \
			auto *pValue = luabind::object_cast_nothrow<handlename *>(o, static_cast<handlename *>(nullptr));                                                                                                                                                                                    \
			return (pValue != nullptr) ? true : false;                                                                                                                                                                                                                                           \
		}                                                                                                                                                                                                                                                                                        \
		static inline classname *Get##localname(lua_State *l, int n)                                                                                                                                                                                                                             \
		{                                                                                                                                                                                                                                                                                        \
			luabind::object o(luabind::from_stack(l, n));                                                                                                                                                                                                                                        \
			auto *pValue = luabind::object_cast_nothrow<handlename *>(o, static_cast<handlename *>(nullptr));                                                                                                                                                                                    \
			if(pValue == nullptr || pValue->IsValid() == false)                                                                                                                                                                                                                                  \
				return nullptr;                                                                                                                                                                                                                                                                  \
			return pValue->get();                                                                                                                                                                                                                                                                \
		}                                                                                                                                                                                                                                                                                        \
	};

#endif
