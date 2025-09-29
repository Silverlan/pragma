// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"


export module pragma.server.entities.components.lua;

import pragma.server.entities.components.entity;

export namespace pragma {
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

export namespace pragma::lua {
	using SLuaBaseEntityComponentHolder = HandleHolder<SLuaBaseEntityComponent>;
};
