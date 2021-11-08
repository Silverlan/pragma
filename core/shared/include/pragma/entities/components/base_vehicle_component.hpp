/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_VEHICLE_COMPONENT_HPP__
#define __BASE_VEHICLE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/physics/vehicle.hpp"

namespace pragma
{
	class BaseWheelComponent;
	namespace physics {class IConvexShape; struct VehicleCreateInfo;};
	class DLLNETWORK BaseVehicleComponent
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_DRIVER_ENTERED;
		static ComponentEventId EVENT_ON_DRIVER_EXITED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent);
		enum class StateFlags : uint32_t
		{
			None = 0u,
			HasDriver = 1u,
			SteeringWheelInitialized = HasDriver<<1u
		};

		virtual void Initialize() override;
		virtual void OnRemove() override;
		BaseEntity *GetDriver();
		bool HasDriver() const;
		virtual void ClearDriver();
		virtual void SetDriver(BaseEntity *ent);
		uint8_t GetWheelCount() const;

		virtual void OnTick(double tDelta) override;

		void SetupVehicle(const pragma::physics::VehicleCreateInfo &createInfo,const std::vector<std::string> &wheelModels);

		BaseEntity *GetSteeringWheel();
		float GetSpeedKmh() const;
		float GetSteeringFactor() const;

		virtual void SetupSteeringWheel(const std::string &mdl,umath::Degree maxSteeringAngle);

		physics::IVehicle *GetPhysicsVehicle();
		const physics::IVehicle *GetPhysicsVehicle() const;
	protected:
		BaseVehicleComponent(BaseEntity &ent);
		struct DLLNETWORK WheelData
		{
			WheelData()=default;
			ComponentHandle<pragma::BaseWheelComponent> hWheel = {};
			std::string model = "";
		};
		pragma::NetEventId m_netEvSteeringWheelModel = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetDriver = pragma::INVALID_NET_EVENT;
		std::vector<WheelData> m_wheels = {};
		physics::VehicleCreateInfo m_vhcCreateInfo = {};
		util::TSharedHandle<physics::IVehicle> m_physVehicle = nullptr;
		std::string m_steeringWheelMdl;
		EntityHandle m_steeringWheel = {};
		CallbackHandle m_cbSteeringWheel = {};
		EntityHandle m_driver = {};
		StateFlags m_stateFlags = StateFlags::None;
		float m_maxSteeringWheelAngle = 0.f;
		void InitializeVehiclePhysics(PHYSICSTYPE type,BasePhysicsComponent::PhysFlags flags);
		void DestroyVehiclePhysics();
		virtual BaseWheelComponent *CreateWheelEntity(uint8_t wheelIndex);
		void InitializeWheelEntities();
		void InitializeSteeringWheel();
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseVehicleComponent::StateFlags)

#endif
