// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include <pragma/entities/components/base_game_component.hpp>
#include "pragma/entities/c_baseentity.h"

export module pragma.client.entities.components.game;

export namespace pragma {
	class DLLCLIENT CGameComponent final : public BaseGameComponent {
	  public:
		CGameComponent(BaseEntity &ent) : BaseGameComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
		void UpdateFrame(CCameraComponent *cam);
		void UpdateCamera(CCameraComponent *cam);
	  private:
	};
};

export class DLLCLIENT CGameEntity : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
