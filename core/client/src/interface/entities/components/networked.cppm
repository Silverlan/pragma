// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.networked;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CNetworkedComponent final : public BaseNetworkedComponent, public CBaseNetComponent {
	  public:
		CNetworkedComponent(ecs::BaseEntity &ent) : BaseNetworkedComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
