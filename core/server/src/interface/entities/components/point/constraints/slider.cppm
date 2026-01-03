// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.point.constraints.slider;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SPointConstraintSliderComponent final : public BasePointConstraintSliderComponent, public SBaseNetComponent {
		  public:
			SPointConstraintSliderComponent(ecs::BaseEntity &ent) : BasePointConstraintSliderComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER PointConstraintSlider : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
