// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.lua;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CLuaBaseEntityComponent final : public BaseLuaBaseEntityComponent, public CBaseSnapshotComponent {
	  public:
		CLuaBaseEntityComponent(ecs::BaseEntity &ent);

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveSnapshotData(NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override;
		virtual bool ShouldTransmitSnapshotData() const override;
	  protected:
		virtual void InvokeNetEventHandle(const std::string &methodName, NetPacket &packet, BasePlayerComponent *pl) override;
	};
};

export namespace pragma::LuaCore {
	using CLuaBaseEntityComponentHolder = HandleHolder<CLuaBaseEntityComponent>;
};
