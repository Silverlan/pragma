// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.point_constraint_slider;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CPointConstraintSliderComponent final : public BasePointConstraintSliderComponent, public CBaseNetComponent {
		  public:
			CPointConstraintSliderComponent(ecs::BaseEntity &ent) : BasePointConstraintSliderComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CPointConstraintSlider : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
