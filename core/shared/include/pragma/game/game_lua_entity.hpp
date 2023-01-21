/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
	BaseEntity *el = nullptr;
#ifndef LUABIND_NO_EXCEPTIONS
	try
	{
#endif
		r = (*o)();
		auto *elLua = luabind::object_cast<TLuaEntity*>(r);
		auto *holder = luabind::object_cast<THandle*>(r);
		if(elLua && holder)
		{
			elLua->SetupLua(r,classname);
			holder->SetHandle(util::weak_shared_handle_cast<BaseEntity,TLuaEntity>(elLua->GetHandle()));
			el = elLua;
		}
		else
		{
			Con::csv<<"WARNING: Unable to create lua entity '"<<classname<<"': Lua class is not derived from valid entity base!"<<Con::endl;
			return nullptr;
		}
#ifndef LUABIND_NO_EXCEPTIONS
	}
	catch(luabind::error&)
	{
		Lua::HandleLuaError(GetLuaState());
		return nullptr;
	}
#endif
	if(!el)
	{
		Con::cwar<<"Unable to create lua entity '"<<classname<<"'!"<<Con::endl;
		return nullptr;
	}
	oClass = *o;
	return el;
}

#endif
