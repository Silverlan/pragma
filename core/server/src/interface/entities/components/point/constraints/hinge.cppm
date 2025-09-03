// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/constraints/point_constraint_hinge.h"
#include "pragma/entities/components/s_entity_component.hpp"

export module pragma.server.entities.components.point.constraints.hinge;

export {
	namespace pragma {
		class DLLSERVER SPointConstraintHingeComponent final : public BasePointConstraintHingeComponent, public SBaseNetComponent {
		public:
			using BasePointConstraintHingeComponent::BasePointConstraintHingeComponent;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER PointConstraintHinge : public SBaseEntity {
	protected:
	public:
		virtual void Initialize() override;
	};
};
