// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.lua;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SLuaBaseEntityComponent final : public BaseLuaBaseEntityComponent, public SBaseSnapshotComponent {
	  public:
		SLuaBaseEntityComponent(ecs::BaseEntity &ent);

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual Bool ReceiveNetEvent(BasePlayerComponent &pl, NetEventId, NetPacket &packet) override;
		virtual void SendSnapshotData(NetPacket &packet, BasePlayerComponent &pl) override;
		virtual bool ShouldTransmitNetData() const override;
		virtual bool ShouldTransmitSnapshotData() const override;

		virtual void OnMemberValueChanged(uint32_t memberIdx) override;
	  protected:
		virtual void InvokeNetEventHandle(const std::string &methodName, NetPacket &packet, BasePlayerComponent *pl) override;
	};
};

export namespace pragma::LuaCore {
	using SLuaBaseEntityComponentHolder = HandleHolder<SLuaBaseEntityComponent>;
};
