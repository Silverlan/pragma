// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
