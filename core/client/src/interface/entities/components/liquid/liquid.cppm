// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.liquid;

export import :entities.base_entity;
export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CLiquidComponent final : public BaseFuncLiquidComponent, public CBaseNetComponent {
	  public:
		CLiquidComponent(ecs::BaseEntity &ent);
		virtual ~CLiquidComponent() override;
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		void UpdateSurfaceSimulator();

		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void OnEntitySpawn() override;
		void SetupWater();
		bool m_bUsingClientsideSimulation = false;
		CallbackHandle m_cbClientSimulatorUpdate = {};
		CallbackHandle m_cbGameInitialized = {};
	};
};

export class DLLCLIENT CFuncWater : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
