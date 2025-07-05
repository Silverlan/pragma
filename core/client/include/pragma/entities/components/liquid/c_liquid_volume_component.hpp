// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LIQUID_VOLUME_COMPONENT_HPP__
#define __C_LIQUID_VOLUME_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/liquid/base_liquid_volume_component.hpp>

namespace pragma {
	class DLLCLIENT CLiquidVolumeComponent final : public BaseLiquidVolumeComponent, public CBaseNetComponent {
	  public:
		CLiquidVolumeComponent(BaseEntity &ent) : BaseLiquidVolumeComponent(ent) {}
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
