/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_GAME_COMPONENT_HPP__
#define __C_GAME_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_game_component.hpp>

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
