// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.radius;

export import :entities.components.entity;

export namespace pragma {
	namespace cRadiusComponent {
		using namespace baseRadiusComponent;
	}
	class DLLCLIENT CRadiusComponent final : public BaseRadiusComponent, public CBaseNetComponent {
	  public:
		CRadiusComponent(ecs::BaseEntity &ent);

		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override;
	};
};
