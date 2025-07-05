// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __GAME_ENTITY_COMPONENT_HPP__
#define __GAME_ENTITY_COMPONENT_HPP__

#include <pragma/game/game.h>

template<class TComponent, class THolder>
pragma::BaseEntityComponent *Game::CreateLuaEntityComponent(BaseEntity &ent, std::string classname)
{
	auto *o = m_luaEnts->GetComponentClassObject(classname);
	if(o == nullptr)
		return nullptr;
	luabind::object r;
	pragma::BaseEntityComponent *el = nullptr;
#ifndef LUABIND_NO_EXCEPTIONS
	try {
#endif
		r = (*o)(ent.GetLuaObject());

		auto *elLua = luabind::object_cast<TComponent *>(r);
		auto *holder = luabind::object_cast<THolder *>(r);
		if(elLua && holder) {
			elLua->SetupLua(r);
			holder->SetHandle(util::weak_shared_handle_cast<pragma::BaseEntityComponent, TComponent>(elLua->GetHandle()));
			el = elLua;
		}
		else {
			Con::csv << "WARNING: Unable to create lua entity component '" << classname << "': Lua class is not derived from valid entity component base!" << Con::endl;
			return nullptr;
		}
#ifndef LUABIND_NO_EXCEPTIONS
	}
	catch(luabind::error &) {
		Lua::HandleLuaError(GetLuaState());
		return nullptr;
	}
#endif
	if(!r) {
		Con::cwar << "Unable to create lua entity component '" << classname << "'!" << Con::endl;
		return nullptr;
	}
	return el;
}

#endif
