#ifndef __GAME_LUA_ENTITY_HPP__
#define __GAME_LUA_ENTITY_HPP__

#include <pragma/game/game.h>

template<class TLuaEntity,class THandle>
	BaseEntity *Game::CreateLuaEntity(std::string classname,luabind::object &oClass,bool bLoadIfNotExists)
{
	auto *o = m_luaEnts->GetClassObject(classname);
	if(o == nullptr)
	{
		if(bLoadIfNotExists && LoadLuaEntityByClass(classname) == true)
			return CreateLuaEntity<TLuaEntity,THandle>(classname,oClass,false);
		return nullptr;
	}
	luabind::object r;
#ifndef LUABIND_NO_EXCEPTIONS
	try
	{
#endif
		r = (*o)();
#ifndef LUABIND_NO_EXCEPTIONS
	}
	catch(luabind::error&)
	{
		Lua::HandleLuaError(GetLuaState());
		return nullptr;
	}
#endif
	if(!r)
	{
		Con::cwar<<"WARNING: Unable to create lua entity '"<<classname<<"'!"<<Con::endl;
		return nullptr;
	}
	BaseEntity *ent = nullptr;
	if(luabind::object_cast_nothrow<THandle*>(r))
		ent = new TLuaEntity(r,classname);
	else
	{
		Con::cwar<<"WARNING: Unable to create lua entity '"<<classname<<"': Lua class is not derived from valid entity base!"<<Con::endl;
		return nullptr;
	}
	oClass = *o;
	return ent;
}

#endif
