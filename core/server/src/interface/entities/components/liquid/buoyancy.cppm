// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.liquid_buoyancy;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SBuoyancyComponent final : public BaseBuoyancyComponent, public SBaseNetComponent {
	  public:
		SBuoyancyComponent(ecs::BaseEntity &ent) : BaseBuoyancyComponent(ent) {}
		virtual void Initialize() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
