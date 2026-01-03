// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.toggle;

export import :entities.components.entity;

export namespace pragma {
	namespace cToggleComponent {
		using namespace baseToggleComponent;
	}
	class DLLCLIENT CToggleComponent final : public BaseToggleComponent, public CBaseNetComponent {
	  public:
		CToggleComponent(ecs::BaseEntity &ent) : BaseToggleComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(UInt32 eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
