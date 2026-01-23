// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.health;

import :entities.components.entity;

export namespace pragma {
	namespace sHealthComponent {
		using namespace baseHealthComponent;
	}
	class DLLSERVER SHealthComponent final : public BaseHealthComponent, public SBaseNetComponent {
	  public:
		SHealthComponent(ecs::BaseEntity &ent) : BaseHealthComponent(ent) {}
		virtual void SetHealth(unsigned short health) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
