/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_GAME_COMPONENT_HPP__
#define __S_GAME_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include <pragma/entities/components/base_game_component.hpp>

namespace pragma {
	class DLLSERVER SGameComponent final : public BaseGameComponent {
	  public:
		SGameComponent(BaseEntity &ent) : BaseGameComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER SGameEntity : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
