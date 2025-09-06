// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/point/constraints/point_constraint_dof.h"
#include "pragma/entities/components/s_entity_component.hpp"

export module pragma.server.entities.components.point.constraints.dof;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SPointConstraintDoFComponent final : public BasePointConstraintDoFComponent, public SBaseNetComponent {
		public:
			using BasePointConstraintDoFComponent::BasePointConstraintDoFComponent;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER PointConstraintDoF : public SBaseEntity {
	protected:
	public:
		virtual void Initialize() override;
	};
};
