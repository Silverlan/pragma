// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.liquid_buoyancy;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CBuoyancyComponent final : public BaseBuoyancyComponent, public CBaseNetComponent {
	  public:
		CBuoyancyComponent(ecs::BaseEntity &ent) : BaseBuoyancyComponent(ent) {}
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
