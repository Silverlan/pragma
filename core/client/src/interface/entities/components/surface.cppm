// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.surface;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CSurfaceComponent final : public BaseSurfaceComponent, public CBaseNetComponent {
	  public:
		CSurfaceComponent(ecs::BaseEntity &ent) : BaseSurfaceComponent(ent) {}

		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
