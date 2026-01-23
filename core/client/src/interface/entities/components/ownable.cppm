// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.ownable;

export import :entities.components.entity;

export namespace pragma {
	namespace cOwnableComponent {
		using namespace baseOwnableComponent;
	}
	class DLLCLIENT COwnableComponent final : public BaseOwnableComponent, public CBaseNetComponent {
	  public:
		COwnableComponent(ecs::BaseEntity &ent) : BaseOwnableComponent(ent) {}

		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
	};
};
