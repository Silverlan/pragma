// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.point.constraints.hinge;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SPointConstraintHingeComponent final : public BasePointConstraintHingeComponent, public SBaseNetComponent {
		  public:
			using BasePointConstraintHingeComponent::BasePointConstraintHingeComponent;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER PointConstraintHinge : public SBaseEntity {
	  protected:
	  public:
		virtual void Initialize() override;
	};
};
