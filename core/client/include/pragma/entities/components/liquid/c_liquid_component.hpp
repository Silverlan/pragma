/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LIQUID_COMPONENT_HPP__
#define __C_LIQUID_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/trigger/c_trigger_touch.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/liquid/base_liquid_component.hpp>

class CWaterSurface;
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

class CWaterSurface;
class DLLCLIENT CFuncWater : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
