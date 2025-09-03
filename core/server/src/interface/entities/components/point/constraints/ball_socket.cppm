// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/constraints/point_constraint_ballsocket.h"
#include "pragma/entities/components/s_entity_component.hpp"

export module pragma.server.entities.components.point.constraints.ball_socket;

export {
	namespace pragma {
		class DLLSERVER SPointConstraintBallSocketComponent final : public BasePointConstraintBallSocketComponent, public SBaseNetComponent {
		public:
			using BasePointConstraintBallSocketComponent::BasePointConstraintBallSocketComponent;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER PointConstraintBallSocket : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
