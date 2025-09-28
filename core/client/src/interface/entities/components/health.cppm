// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.client:entities.components.health;

import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CHealthComponent final : public BaseHealthComponent, public CBaseNetComponent {
	  public:
		CHealthComponent(BaseEntity &ent) : BaseHealthComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
