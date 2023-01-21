/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LDEFINITIONS_H__
#define __LDEFINITIONS_H__
#include <pragma/lua/luaapi.h>
#include "pragma/networkdefinitions.h"
#include <sharedutils/util_shared_handle.hpp>
#define luaL_checkboolean(L,n)    (luaL_checktype(L, (n), LUA_TBOOLEAN))
#define luaL_checkfunction(L,n)    (luaL_checktype(L, (n), LUA_TFUNCTION))
#define luaL_checklightuserdata(L,n)    (luaL_checktype(L, (n), LUA_TLIGHTUSERDATA))
#define luaL_checknil(L,n)    (luaL_checktype(L, (n), LUA_TNIL))
#define luaL_checktable(L,n)    (luaL_checktype(L, (n), LUA_TTABLE))
#define luaL_checkthread(L,n)    (luaL_checktype(L, (n), LUA_TTHREAD))
#define luaL_checkuserdata(L,n)    (luaL_checktype(L, (n), LUA_TUSERDATA))

namespace Lua
{
	static void TypeError(const luabind::object &o,Type type);
	DLLNETWORK Lua::Type GetType(const luabind::object &o);
	DLLNETWORK void CheckType(const luabind::object &o,Type type);
};

using LuaTableObject = luabind::object;
using LuaClassObject = luabind::object;
using LuaFunctionObject = luabind::object;

class BaseLuaObj;
namespace Lua
{
	DLLNETWORK void PushObject(lua_State *l,BaseLuaObj *o);
	
	template<class TType>
		bool CheckHandle(lua_State *l,const util::TSharedHandle<TType> &handle);
	template<class TType>
		bool CheckHandle(lua_State *l,const util::TWeakSharedHandle<TType> &handle);
	template<class TType>
		bool CheckHandle(lua_State *l,const TType *value);
	template<class TType>
		TType &CheckHandle(lua_State *l,const int32_t idx);
};

template<class TType>
	bool Lua::CheckHandle(lua_State *l,const util::TSharedHandle<TType> &handle)
{
	if(handle.IsExpired())
	{
		Lua::PushString(l,"Attempted to use a NULL handle");
		lua_error(l);
		return false;
	}
	return true;
}

template<class TType>
	bool Lua::CheckHandle(lua_State *l,const util::TWeakSharedHandle<TType> &handle)
{
	if(handle.IsExpired())
	{
		Lua::PushString(l,"Attempted to use a NULL handle");
		lua_error(l);
		return false;
	}
	return true;
}
template<class TType>
	TType &Lua::CheckHandle(lua_State *l,const int32_t idx)
{
	auto *handle = CheckPtr<TType>(l,idx);
	if(handle == nullptr)
	{
		Lua::PushString(l,"Attempted to use a NULL handle");
		lua_error(l);
		// Unreachable
	}
	return *handle;
}

template<class TType>
	bool Lua::CheckHandle(lua_State *l,const TType *value)
{
	if(value == nullptr)
	{
		Lua::PushString(l,"Attempted to use a NULL handle");
		lua_error(l);
		// Unreachable
	}
	return true;
}

inline int lua_createreference(lua_State *l,int index)
{
	lua_pushvalue(l,index);
	return luaL_ref(l,LUA_REGISTRYINDEX);
}

inline void lua_removereference(lua_State *l,int index)
{
	luaL_unref(l,LUA_REGISTRYINDEX,index);
}

namespace Lua
{
	enum class StatusCode : decltype(LUA_OK);
	DLLNETWORK Lua::StatusCode Execute(lua_State *l,const std::function<Lua::StatusCode(int(*traceback)(lua_State*))> &target);
	DLLNETWORK void Execute(lua_State *l,const std::function<void(int(*traceback)(lua_State*),void(*syntaxHandle)(lua_State*,Lua::StatusCode))> &target);
	DLLNETWORK void HandleLuaError(lua_State *l);
	DLLNETWORK void HandleLuaError(lua_State *l,Lua::StatusCode s);
	DLLNETWORK std::string GetErrorMessagePrefix(lua_State *l);
	template<typename T>
		void table_to_vector(lua_State *l,const luabind::object &t,int32_t tableStackIndex,std::vector<T> &outData);
	template<typename T>
		std::vector<T> table_to_vector(lua_State *l,const luabind::object &t,int32_t tableStackIndex);
	template<typename T>
		luabind::object vector_to_table(lua_State *l,const std::vector<T> &data);

	template<typename T0,typename T1>
		void table_to_map(lua_State *l,const luabind::object &t,int32_t tableStackIndex,std::unordered_map<T0,T1> &outData);
	template<typename T0,typename T1>
		std::unordered_map<T0,T1> table_to_map(lua_State *l,const luabind::object &t,int32_t tableStackIndex);
	template<typename T0,typename T1>
		luabind::object map_to_table(lua_State *l,const std::unordered_map<T0,T1> &data);
};

template<typename T>
	void Lua::table_to_vector(lua_State *l,const luabind::object &t,int32_t tableStackIndex,std::vector<T> &outData)
{
	auto n = Lua::GetObjectLength(l,tableStackIndex);
	outData.reserve(outData.size() +n);
	for(auto it=luabind::iterator{t},end=luabind::iterator{};it!=end;++it)
	{
		auto val = luabind::object_cast_nothrow<T>(*it,T{});
		outData.push_back(val);
	}
}

template<typename T>
	std::vector<T> Lua::table_to_vector(lua_State *l,const luabind::object &t,int32_t tableStackIndex)
{
	std::vector<T> result {};
	table_to_vector(l,t,tableStackIndex,result);
	return result;
}

template<typename T>
	luabind::object Lua::vector_to_table(lua_State *l,const std::vector<T> &data)
{
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto &v : data)
		t[idx++] = v;
	return t;
}

template<typename T0,typename T1>
	void Lua::table_to_map(lua_State *l,const luabind::object &t,int32_t tableStackIndex,std::unordered_map<T0,T1> &outData)
{
	for(auto it=luabind::iterator{t},end=luabind::iterator{};it!=end;++it)
	{
		auto key = luabind::object_cast_nothrow<T0>(it.key(),T0{});
		auto val = luabind::object_cast_nothrow<T1>(*it,T1{});
		outData[key] = val;
	}
}

template<typename T0,typename T1>
	std::unordered_map<T0,T1> Lua::table_to_map(lua_State *l,const luabind::object &t,int32_t tableStackIndex)
{
	std::unordered_map<T0,T1> result {};
	table_to_map(l,t,tableStackIndex,result);
	return result;
}

template<typename T0,typename T1>
	luabind::object Lua::map_to_table(lua_State *l,const std::unordered_map<T0,T1> &data)
{
	auto t = luabind::newtable(l);
	for(auto &pair : data)
		t[pair.first] = pair.second;
	return t;
}

#define lua_registerglobalint(global) \
	lua_pushinteger(m_lua,global); \
	lua_setglobal(m_lua,#global);

#define STR(arg) #arg
#define lua_registerglobalenum(en,name) \
	lua_pushinteger(m_lua,int(en::name)); \
	lua_setglobal(m_lua,STR(en##_##name));

#define lua_checkentityret(l,hEnt,ret) \
	if(!hEnt->IsValid()) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL entity"); \
		lua_error(l); \
		return ret; \
	}

#define lua_checkentity(l,hEnt) \
	if(!hEnt->IsValid()) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL entity"); \
		lua_error(l); \
		return; \
	}

#define lua_checktimer(l,hTimer) \
	{ \
		if(hTimer.IsValid() == false) \
		{ \
			lua_pushstring(l,"Attempted to use a NULL timer"); \
			lua_error(l); \
			return ; \
		} \
	}

#define lua_pushentity(luastate,ent) \
	{ \
		ent->GetLuaObject().push(luastate); \
	}

#define lua_pushtablecfunction(l,tablename,funcname,cfuncref) \
	lua_getglobal(l,tablename); \
	if(lua_istable(l,-1)) \
	{ \
		int top = lua_gettop(l); \
		lua_pushstring(l,funcname); \
		lua_pushcfunction(l,cfuncref); \
		lua_settable(l,top); \
	} \
	lua_pop(l,1);

#define lua_gettablefunction(lstate,tablename,funcname,oncall) \
	lua_getglobal(lstate,tablename); \
	if(lua_istable(lstate,-1)) \
	{ \
		lua_getfield(lstate,-1,funcname); \
		if(lua_isfunction(lstate,-1)) \
		{ \
			oncall \
		} \
	} \
	lua_pop(l,1);

#define lua_callfunction(nwstate,func) \
	{ \
		lua_State *l = nwstate->GetLuaState(); \
		lua_rawgeti(l,LUA_REGISTRYINDEX,func); \
		luaerror(nwstate,lua_pcall(l,0,0,0)); \
	}

#define lua_calltablefunction(lstate,tablename,funcname,numreturn,oncall) \
	lua_getglobal(lstate,tablename); \
	if(lua_istable(lstate,-1)) \
	{ \
		lua_getfield(lstate,-1,funcname); \
		if(lua_isfunction(lstate,-1)) \
		{ \
			if(lua_pcall((lstate),(0),(numreturn),(0)) == 0) \
			{ \
				oncall \
			} \
		} \
	} \
	lua_pop(l,1);

#define COMMA ,
#define lua_bind(data) \
	luabind::module(m_lua) \
	[ \
		data \
	];

inline const char *lua_gettype(lua_State *l,int n)
{
	const char *arg = lua_tostring(l,n);
	if(arg == NULL)
	{
		int type = lua_type(l,n);
		switch(type)
		{
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
}

namespace Lua
{
	template<typename T>
		concept is_trivial_type = std::is_same_v<T,bool> || std::is_arithmetic_v<T> || util::is_string<T>::value;
	template<typename T,typename=std::enable_if_t<is_trivial_type<T>>>
		T Check(lua_State *l,int32_t n)
	{
		if constexpr(std::is_same_v<T,bool>)
			return Lua::CheckBool(l,n);
		else if constexpr(std::is_integral_v<T>)
			return Lua::CheckInt(l,n);
		else if constexpr(std::is_arithmetic_v<T>)
			return Lua::CheckNumber(l,n);
		else// if constexpr(util::is_string<T>::value)
			return Lua::CheckString(l,n);
	}
	template<typename T,typename=std::enable_if_t<!is_trivial_type<T>>>
		T &Check(lua_State *l,int32_t n)
	{
		Lua::CheckUserData(l,n);
		luabind::object o(luabind::from_stack(l,n));
		auto *pValue = luabind::object_cast_nothrow<T*>(o,static_cast<T*>(nullptr));
		if(pValue == nullptr)
		{
			std::string err = std::string(typeid(T).name()) +" expected, got ";
			err += lua_gettype(l,n);
			luaL_argerror(l,n,err.c_str());
		}
		return *pValue;
	}

	template<typename T,typename=std::enable_if_t<!std::is_arithmetic<T>::value>>
		T *CheckPtr(lua_State *l,int32_t n)
	{
		Lua::CheckUserData(l,n);
		luabind::object o(luabind::from_stack(l,n));
		return luabind::object_cast_nothrow<T*>(o,static_cast<T*>(nullptr));
	}

	template<typename T>
		bool IsType(lua_State *l,int32_t n)
	{
		if constexpr(std::is_same_v<T,bool>)
			return Lua::IsBool(l,n);
		else if constexpr(std::is_arithmetic_v<T>)
			return Lua::IsNumber(l,n);
		else if constexpr(util::is_string<T>::value)
			return Lua::IsString(l,n);
		else
		{
			if(!lua_isuserdata(l,n))
				return false;
			luabind::object o(luabind::from_stack(l,n));
			auto *pValue = luabind::object_cast_nothrow<T*>(o,static_cast<T*>(nullptr));
			return (pValue != nullptr) ? true : false;
		}
	}
};

#define lua_registercheck(type,cls) \
	inline cls *_lua_##type##_check(lua_State *l,int n) \
	{ \
		luaL_checkuserdata(l,n); \
		luabind::object o(luabind::from_stack(l,n)); \
		auto *pValue = luabind::object_cast_nothrow<cls*>(o,static_cast<cls*>(nullptr)); \
		if(pValue == nullptr) \
		{ \
			std::string err = #type " expected, got "; \
			err += lua_gettype(l,n); \
			luaL_argerror(l,n,err.c_str()); \
		} \
		return pValue; \
	} \
	inline bool _lua_is##type(lua_State *l,int n) \
	{ \
		if(!lua_isuserdata(l,n)) \
			return false; \
		luabind::object o(luabind::from_stack(l,n)); \
		auto *pValue = luabind::object_cast_nothrow<cls*>(o,static_cast<cls*>(nullptr)); \
		return (pValue != nullptr) ? true : false; \
	} \
	inline cls *_lua_##type##_get(lua_State *l,int n) \
	{ \
		luabind::object o(luabind::from_stack(l,n)); \
		return luabind::object_cast_nothrow<cls*>(o,static_cast<cls*>(nullptr)); \
	} \
	namespace Lua \
	{ \
		static cls *(&Check##type)(lua_State*,int) = _lua_##type##_check; \
		static bool (&Is##type)(lua_State*,int) = _lua_is##type; \
		static cls *(&To##type)(lua_State*,int) = _lua_##type##_get; \
	};

#define lua_registercheck_shared_ptr(type,cls) \
	inline cls *_lua_##type##_check(lua_State *l,int n) \
	{ \
		luaL_checkuserdata(l,n); \
		luabind::object o(luabind::from_stack(l,n)); \
		auto *pValue = luabind::object_cast_nothrow<std::shared_ptr<cls>*>(o,static_cast<std::shared_ptr<cls>*>(nullptr)); \
		if(pValue == nullptr || pValue->get() == NULL) \
		{ \
			std::string err = #type " expected, got "; \
			err += lua_gettype(l,n); \
			luaL_argerror(l,n,err.c_str()); \
		} \
		return pValue->get(); \
	} \
	inline bool _lua_is##type(lua_State *l,int n) \
	{ \
		if(!lua_isuserdata(l,n)) \
			return false; \
		luabind::object o(luabind::from_stack(l,n)); \
		auto *pValue = luabind::object_cast_nothrow<std::shared_ptr<cls>*>(o,static_cast<std::shared_ptr<cls>*>(nullptr)); \
		return (pValue != nullptr && pValue->get() != NULL) ? true : false; \
	} \
	inline cls *_lua_##type##_get(lua_State *l,int n) \
	{ \
		luabind::object o(luabind::from_stack(l,n)); \
		auto *pValue = luabind::object_cast_nothrow<std::shared_ptr<cls>*>(o,static_cast<std::shared_ptr<cls>*>(nullptr)); \
		return pValue->get(); \
	} \
	namespace Lua \
	{ \
		static cls *(&Check##type)(lua_State*,int) = _lua_##type##_check; \
		static bool (&Is##type)(lua_State*,int) = _lua_is##type; \
		static cls *(&To##type)(lua_State*,int) = _lua_##type##_get; \
	};

#define lua_registercheck_inherited_ptr(type,cls,pcls) \
	inline cls *_lua_##type##_check(lua_State *l,int n) \
	{ \
		luaL_checkuserdata(l,n); \
		luabind::object o(luabind::from_stack(l,n)); \
		auto *pValue = luabind::object_cast_nothrow<pcls*>(o,static_cast<pcls*>(nullptr)); \
		if(pValue == nullptr) \
		{ \
			std::string err = #type " expected, got "; \
			err += lua_gettype(l,n); \
			luaL_argerror(l,n,err.c_str()); \
		} \
		return pValue; \
	} \
	inline bool _lua_is##type(lua_State *l,int n) \
	{ \
		if(!lua_isuserdata(l,n)) \
			return false; \
		luabind::object o(luabind::from_stack(l,n)); \
		auto *pValue = luabind::object_cast_nothrow<pcls*>(o,static_cast<pcls*>(nullptr)); \
		return (pValue != nullptr) ? true : false; \
	} \
	inline cls *_lua_##type##_get(lua_State *l,int n) \
	{ \
		luabind::object o(luabind::from_stack(l,n)); \
		auto *pValue = luabind::object_cast_nothrow<pcls*>(o,static_cast<pcls*>(nullptr)); \
		return pValue; \
	} \
	namespace Lua \
	{ \
		static cls *(&Check##type)(lua_State*,int) = _lua_##type##_check; \
		static bool (&Is##type)(lua_State*,int) = _lua_is##type; \
		static cls *(&To##type)(lua_State*,int) = _lua_##type##_get; \
	};

#define LUA_SETUP_HANDLE_CHECK(localname,classname,handlename) \
	namespace Lua \
	{ \
		static inline classname *Check##localname(lua_State *l,int n) \
		{ \
			luaL_checkuserdata(l,n); \
			luabind::object o(luabind::from_stack(l,n)); \
			auto *pValue = luabind::object_cast_nothrow<handlename*>(o,static_cast<handlename*>(nullptr)); \
			if(pValue == nullptr) \
			{ \
				std::string err = #classname " expected, got "; \
				err += lua_gettype(l,n); \
				luaL_argerror(l,n,err.c_str()); \
			} \
			if(!pValue->IsValid()) \
			{ \
				std::string err = "Attempted to use a NULL "; \
				err += #localname; \
				std::transform(err.begin(),err.end(),err.begin(),::tolower); \
				lua_pushstring(l,err.c_str()); \
				lua_error(l); \
			} \
			return pValue->get(); \
		} \
		static inline handlename *Check##localname##Handle(lua_State *l,int n) \
		{ \
			luaL_checkuserdata(l,n); \
			luabind::object o(luabind::from_stack(l,n)); \
			auto *pValue = luabind::object_cast_nothrow<handlename*>(o,static_cast<handlename*>(nullptr)); \
			if(pValue == nullptr) \
			{ \
				std::string err = #classname " expected, got "; \
				err += lua_gettype(l,n); \
				luaL_argerror(l,n,err.c_str()); \
			} \
			return pValue; \
		} \
		static inline bool Is##localname(lua_State *l,int n) \
		{ \
			if(!lua_isuserdata(l,n)) \
				return false; \
			luabind::object o(luabind::from_stack(l,n)); \
			auto *pValue = luabind::object_cast_nothrow<handlename*>(o,static_cast<handlename*>(nullptr)); \
			return (pValue != nullptr) ? true : false; \
		} \
		static inline classname *Get##localname(lua_State *l,int n) \
		{ \
			luabind::object o(luabind::from_stack(l,n)); \
			auto *pValue = luabind::object_cast_nothrow<handlename*>(o,static_cast<handlename*>(nullptr)); \
			if(pValue == nullptr || pValue->IsValid() == false) \
				return nullptr; \
			return pValue->get(); \
		} \
	};

#endif