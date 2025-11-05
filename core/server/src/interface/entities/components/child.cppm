// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.child;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SChildComponent final : public BaseChildComponent, public SBaseNetComponent {
	  public:
		SChildComponent(pragma::ecs::BaseEntity &ent) : BaseChildComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void OnParentChanged(pragma::ecs::BaseEntity *parent) override;
	};
};
