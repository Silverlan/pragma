// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_OWNABLE_COMPONENT_HPP__
#define __S_OWNABLE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_ownable_component.hpp>
#include <pragma/lua/luaapi.h>

namespace pragma {
	class DLLSERVER SOwnableComponent final : public BaseOwnableComponent, public SBaseNetComponent {
	  public:
		SOwnableComponent(BaseEntity &ent) : BaseOwnableComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
		using BaseOwnableComponent::SetOwner;
	  protected:
		virtual void SetOwner(BaseEntity *owner) override;
	};
};

#endif
