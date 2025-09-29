// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:entities.components.game;

import :entities.components.camera;

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
