// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.color;

export import :entities.components.entity;

export namespace pragma {
	namespace cColorComponent {
		using namespace baseColorComponent;
	}
	class DLLCLIENT CColorComponent final : public BaseColorComponent, public CBaseNetComponent {
	  public:
		CColorComponent(ecs::BaseEntity &ent) : BaseColorComponent(ent) {}

		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
