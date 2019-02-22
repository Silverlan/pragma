#ifndef __LUAFUNCTION_H__
#define __LUAFUNCTION_H__

#include "pragma/networkdefinitions.h"
#include <memory>
#include <string>

namespace luabind{class object;}
class DLLNETWORK LuaFunction
{
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
	template<class T,typename... TARGS>
		T Call(TARGS ...args);
	template<class T,typename... TARGS>
		bool Call(T *ret,TARGS ...args);
};

#endif