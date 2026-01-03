// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.liquid_volume;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CLiquidVolumeComponent final : public BaseLiquidVolumeComponent, public CBaseNetComponent {
	  public:
		CLiquidVolumeComponent(ecs::BaseEntity &ent) : BaseLiquidVolumeComponent(ent) {}
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
