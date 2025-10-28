// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"


export module pragma.server.entities.components.ownable;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SOwnableComponent final : public BaseOwnableComponent, public SBaseNetComponent {
	  public:
		SOwnableComponent(pragma::ecs::BaseEntity &ent) : BaseOwnableComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
		using BaseOwnableComponent::SetOwner;
	  protected:
		virtual void SetOwner(pragma::ecs::BaseEntity *owner) override;
	};
};
