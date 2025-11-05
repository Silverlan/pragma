// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.point.constraints.fixed;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SPointConstraintFixedComponent final : public BasePointConstraintFixedComponent, public SBaseNetComponent {
		  public:
			using BasePointConstraintFixedComponent::BasePointConstraintFixedComponent;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER PointConstraintFixed : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
