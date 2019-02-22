#ifndef __LUA_CALLBACK_HANDLER_H__
#define __LUA_CALLBACK_HANDLER_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luacallback.h"

class DLLNETWORK LuaCallbackHandler
{
protected:
	std::unordered_map<std::string,std::vector<CallbackHandle>> m_luaCallbacks;
public:
	// Lua Callbacks
	CallbackHandle AddLuaCallback(std::string identifier,const luabind::object &o);
	std::vector<CallbackHandle> *GetLuaCallbacks(std::string identifier);
	void CallLuaCallbacks(const std::string &name);
	template<class T,typename... TARGS>
		T CallLuaCallbacks(std::string name,TARGS ...args);
	template<class T,typename... TARGS>
		CallbackReturnType CallLuaCallbacks(std::string name,T *ret,TARGS ...args);
};

template<class T,typename... TARGS>
	T LuaCallbackHandler::CallLuaCallbacks(std::string name,TARGS ...args)
{
	ustring::to_lower(name);
	auto it = m_luaCallbacks.find(name);
	if(it == m_luaCallbacks.end())
		return T();
	auto &callbacks = it->second;
	for(auto it=callbacks.begin();it!=callbacks.end();++it)
	{
		auto &hCallback = *it;
		if(hCallback.IsValid())
		{
			auto *f = static_cast<LuaCallback*>(hCallback.get());
			if(std::is_same<T,void>::value == true)
				f->Call<T,TARGS...>(args...);
			else
				return f->Call<T,TARGS...>(args...);
		}
	}
	return T();
}
template<class T,typename... TARGS>
	CallbackReturnType LuaCallbackHandler::CallLuaCallbacks(std::string name,T *ret,TARGS ...args)
{
	ustring::to_lower(name);
	auto it = m_luaCallbacks.find(name);
	if(it == m_luaCallbacks.end())
		return CallbackReturnType::NoReturnValue;
	auto &callbacks = it->second;
	for(auto it=callbacks.begin();it!=callbacks.end();++it)
	{
		auto &hCallback = *it;
		if(hCallback.IsValid())
		{
			auto *f = static_cast<LuaCallback*>(hCallback.get());
			if(f->Call<T,TARGS...>(ret,args...) == true)
				return CallbackReturnType::HasReturnValue;
		}
	}
	return CallbackReturnType::NoReturnValue;
}

#endif