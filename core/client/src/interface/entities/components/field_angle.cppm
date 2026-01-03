// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.field_angle;

export import :entities.components.entity;

export namespace pragma {
	namespace cFieldAngleComponent {
		using namespace baseFieldAngleComponent;
	}
	class DLLCLIENT CFieldAngleComponent final : public BaseFieldAngleComponent, public CBaseNetComponent {
	  public:
		CFieldAngleComponent(ecs::BaseEntity &ent) : BaseFieldAngleComponent(ent) {}

		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
