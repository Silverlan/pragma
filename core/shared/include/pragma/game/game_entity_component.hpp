/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __GAME_ENTITY_COMPONENT_HPP__
#define __GAME_ENTITY_COMPONENT_HPP__

#include <pragma/game/game.h>

struct BaseLuaBaseEntityComponentHandleWrapper;
template<class TComponent>
	pragma::BaseEntityComponent *Game::CreateLuaEntityComponent(BaseEntity &ent,std::string classname)
{
	auto *o = m_luaEnts->GetComponentClassObject(classname);
	if(o == nullptr)
		return nullptr;
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
		Con::cwar<<"WARNING: Unable to create lua entity component '"<<classname<<"'!"<<Con::endl;
		return nullptr;
	}
	pragma::BaseEntityComponent *pComponent = nullptr;
	if(luabind::object_cast_nothrow<BaseLuaBaseEntityComponentHandleWrapper*>(r,static_cast<BaseLuaBaseEntityComponentHandleWrapper*>(nullptr)))
		pComponent = new TComponent(ent,r);
	else
	{
		Con::cwar<<"WARNING: Unable to create lua entity component '"<<classname<<"': Lua class is not derived from valid component base!"<<Con::endl;
		return nullptr;
	}
	return pComponent;
}

#endif
