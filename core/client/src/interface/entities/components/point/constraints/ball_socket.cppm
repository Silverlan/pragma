// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.point_constraint_ball_socket;

import :entities.base_entity;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CPointConstraintBallSocketComponent final : public BasePointConstraintBallSocketComponent, public CBaseNetComponent {
		public:
			CPointConstraintBallSocketComponent(BaseEntity &ent) : BasePointConstraintBallSocketComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CPointConstraintBallSocket : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
