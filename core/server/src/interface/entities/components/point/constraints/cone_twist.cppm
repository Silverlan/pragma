// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.point.constraints.cone_twist;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SPointConstraintConeTwistComponent final : public BasePointConstraintConeTwistComponent, public SBaseNetComponent {
		  public:
			using BasePointConstraintConeTwistComponent::BasePointConstraintConeTwistComponent;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER PointConstraintConeTwist : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
