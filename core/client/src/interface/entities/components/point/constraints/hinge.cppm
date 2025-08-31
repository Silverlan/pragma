// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/point/constraints/point_constraint_hinge.h"
#include "pragma/entities/components/c_entity_component.hpp"

export module pragma.client.entities.components:constraint_hinge;

export {
	namespace pragma {
		class DLLCLIENT CPointConstraintHingeComponent final : public BasePointConstraintHingeComponent, public CBaseNetComponent {
		public:
			CPointConstraintHingeComponent(BaseEntity &ent) : BasePointConstraintHingeComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CPointConstraintHinge : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
