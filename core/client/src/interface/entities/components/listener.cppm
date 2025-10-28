// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "alsound_listener.hpp"
#include "pragma/lua/core.hpp"


export module pragma.client:entities.components.listener;

import :entities.base_entity;

export namespace pragma {
	class DLLCLIENT CListenerComponent final : public BaseEntityComponent {
	  public:
		CListenerComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		CListenerComponent();
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
		float GetGain();
		void SetGain(float gain);
		virtual void InitializeLuaObject(lua_State *l) override;
	  private:
		al::IListener *m_listener = nullptr;
	};
};

export class DLLCLIENT CListener : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
