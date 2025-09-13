// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LIQUID_COMPONENT_HPP__
#define __C_LIQUID_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/trigger/c_trigger_touch.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/liquid/base_liquid_component.hpp>

namespace pragma {
	class DLLCLIENT CLiquidComponent final : public BaseFuncLiquidComponent, public CBaseNetComponent {
	  public:
		CLiquidComponent(BaseEntity &ent);
		virtual ~CLiquidComponent() override;
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		void UpdateSurfaceSimulator();

		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void OnEntitySpawn() override;
		void SetupWater();
		bool m_bUsingClientsideSimulation = false;
		CallbackHandle m_cbClientSimulatorUpdate = {};
		CallbackHandle m_cbGameInitialized = {};
	};
};

class DLLCLIENT CFuncWater : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
