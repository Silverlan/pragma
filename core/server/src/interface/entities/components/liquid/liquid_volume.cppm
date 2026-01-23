// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.liquid_volume;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SLiquidVolumeComponent final : public BaseLiquidVolumeComponent, public SBaseNetComponent {
	  public:
		SLiquidVolumeComponent(ecs::BaseEntity &ent) : BaseLiquidVolumeComponent(ent) {}
		virtual void Initialize() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
