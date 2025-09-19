// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/liquid/base_liquid_volume_component.hpp>

export module pragma.client.entities.components.liquid_volume;

export namespace pragma {
	class DLLCLIENT CLiquidVolumeComponent final : public BaseLiquidVolumeComponent, public CBaseNetComponent {
	  public:
		CLiquidVolumeComponent(BaseEntity &ent) : BaseLiquidVolumeComponent(ent) {}
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
