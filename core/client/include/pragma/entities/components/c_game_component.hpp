// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_GAME_COMPONENT_HPP__
#define __C_GAME_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_game_component.hpp>
#include "pragma/entities/c_baseentity.h"

namespace pragma {
	class CCameraComponent;
	class DLLCLIENT CGameComponent final : public BaseGameComponent {
	  public:
		CGameComponent(BaseEntity &ent) : BaseGameComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
		void UpdateFrame(CCameraComponent *cam);
		void UpdateCamera(CCameraComponent *cam);
	  private:
	};
};

class DLLCLIENT CGameEntity : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
