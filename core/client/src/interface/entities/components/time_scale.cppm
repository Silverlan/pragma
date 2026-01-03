// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.time_scale;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CTimeScaleComponent final : public BaseTimeScaleComponent, public CBaseNetComponent {
	  public:
		CTimeScaleComponent(ecs::BaseEntity &ent) : BaseTimeScaleComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void ReceiveData(NetPacket &packet) override {}
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
	};
};
