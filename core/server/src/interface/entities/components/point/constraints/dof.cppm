// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.point.constraints.dof;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SPointConstraintDoFComponent final : public BasePointConstraintDoFComponent, public SBaseNetComponent {
		  public:
			using BasePointConstraintDoFComponent::BasePointConstraintDoFComponent;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER PointConstraintDoF : public SBaseEntity {
	  protected:
	  public:
		virtual void Initialize() override;
	};
};
