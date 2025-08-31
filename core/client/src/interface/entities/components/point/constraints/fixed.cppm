// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/point/constraints/point_constraint_fixed.h"
#include "pragma/entities/components/c_entity_component.hpp"

export module pragma.client.entities.components:constraint_fixed;

export {
	namespace pragma {
		class DLLCLIENT CPointConstraintFixedComponent final : public BasePointConstraintFixedComponent, public CBaseNetComponent {
		public:
			CPointConstraintFixedComponent(BaseEntity &ent) : BasePointConstraintFixedComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CPointConstraintFixed : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
