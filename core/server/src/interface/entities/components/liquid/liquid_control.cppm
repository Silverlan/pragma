// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.liquid_control;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SLiquidControlComponent final : public BaseLiquidControlComponent, public SBaseNetComponent {
	  public:
		SLiquidControlComponent(ecs::BaseEntity &ent) : BaseLiquidControlComponent(ent) {}
		virtual void Initialize() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua::State *l) override;

		virtual void CreateSplash(const Vector3 &origin, float radius, float force) override;
	};
};
