// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"



export module pragma.shared:entities.components.base_wheel;

export import :entities.components.base;
export import :physics.vehicle;

export namespace pragma {
	class BaseVehicleComponent;
	class DLLNETWORK BaseWheelComponent : public BaseEntityComponent {
	  public:
		virtual ~BaseWheelComponent() override;
		virtual void Initialize() override;
		util::WeakHandle<pragma::BaseVehicleComponent> GetVehicle();

		void SetupWheel(BaseVehicleComponent &vhc, const pragma::physics::WheelCreateInfo &createInfo, uint8_t wheelId);
	  protected:
		BaseWheelComponent(pragma::ecs::BaseEntity &ent);
		void UpdatePose();
		ComponentHandle<pragma::BaseVehicleComponent> m_vehicle = {};
		uint8_t m_wheelId = 0u;
		physics::WheelCreateInfo m_createInfo = {};
		umath::Transform m_localTransform = {};

		virtual void OnTick(double dt) override;
		void UpdateWheel();
	  private:
		CallbackHandle m_cbOnSpawn = {};
	};
};
