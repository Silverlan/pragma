/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LUA_COMPONENT_HPP__
#define __S_LUA_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/lua/sh_lua_component.hpp>
#include <pragma/lua/handle_holder.hpp>

namespace pragma {
	class DLLSERVER SLuaBaseEntityComponent final : public BaseLuaBaseEntityComponent, public SBaseSnapshotComponent {
	  public:
		SLuaBaseEntityComponent(BaseEntity &ent);

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual Bool ReceiveNetEvent(pragma::BasePlayerComponent &pl, pragma::NetEventId, NetPacket &packet) override;
		virtual void SendSnapshotData(NetPacket &packet, pragma::BasePlayerComponent &pl) override;
		virtual bool ShouldTransmitNetData() const override;
		virtual bool ShouldTransmitSnapshotData() const override;

		virtual void OnMemberValueChanged(uint32_t memberIdx) override;
	  protected:
		virtual void InvokeNetEventHandle(const std::string &methodName, NetPacket &packet, pragma::BasePlayerComponent *pl) override;
	};
};

namespace pragma::lua {
	using SLuaBaseEntityComponentHolder = HandleHolder<SLuaBaseEntityComponent>;
};

#endif
