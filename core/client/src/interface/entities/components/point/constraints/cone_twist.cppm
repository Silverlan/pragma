// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/point/constraints/point_constraint_conetwist.h"

export module pragma.client.entities.components.point_constraint_cone_twist;

export {
	namespace pragma {
		class DLLCLIENT CPointConstraintConeTwistComponent final : public BasePointConstraintConeTwistComponent, public CBaseNetComponent {
		public:
			CPointConstraintConeTwistComponent(BaseEntity &ent) : BasePointConstraintConeTwistComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CPointConstraintConeTwist : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
