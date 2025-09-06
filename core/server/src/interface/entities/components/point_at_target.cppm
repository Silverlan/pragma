// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include <pragma/entities/components/base_point_at_target_component.hpp>
#include <pragma/lua/luaapi.h>

export module pragma.server.entities.components.point_at_target;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SPointAtTargetComponent final : public BasePointAtTargetComponent, public SBaseNetComponent {
	  public:
		SPointAtTargetComponent(BaseEntity &ent) : BasePointAtTargetComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;

		using BasePointAtTargetComponent::SetPointAtTarget;
	  protected:
		virtual void SetPointAtTarget(BaseEntity *ent) override;
	};
};
