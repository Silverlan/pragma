// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_GAMEMODE_COMPONENT_HPP__
#define __C_GAMEMODE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/components/base_gamemode_component.hpp>

namespace pragma {
	class DLLCLIENT CGamemodeComponent final : public BaseGamemodeComponent {
	  public:
		CGamemodeComponent(BaseEntity &ent) : BaseGamemodeComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CGamemode : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
