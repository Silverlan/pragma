#ifndef __LUAOBJ_HANDLER_HPP__
#define __LUAOBJ_HANDLER_HPP__

#include <luasystem.h>
#include <vector>
#include <memory>

class LuaObjHandler
{
public:
	std::weak_ptr<luabind::object> RegisterObject(const luabind::object &o);
	void FreeObject(const luabind::object &o);
protected:
	LuaObjHandler()=default;
	std::vector<std::shared_ptr<luabind::object>> m_objects;
};

#endif
