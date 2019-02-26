#include "luaobj_handler.hpp"

std::weak_ptr<luabind::object> LuaObjHandler::RegisterObject(const luabind::object &o) {m_objects.push_back(std::make_shared<luabind::object>(o)); return m_objects.back();}
void LuaObjHandler::FreeObject(const luabind::object &o)
{
	auto it = std::find_if(m_objects.begin(),m_objects.end(),[&o](const std::shared_ptr<luabind::object> &oOther) {
		return (&o == oOther.get()) ? true : false;
	});
	if(it == m_objects.end())
		return;
	m_objects.erase(it);
}
