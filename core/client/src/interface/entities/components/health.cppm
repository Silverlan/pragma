// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.health;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CHealthComponent final : public BaseHealthComponent, public CBaseNetComponent {
	  public:
		CHealthComponent(ecs::BaseEntity &ent) : BaseHealthComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
