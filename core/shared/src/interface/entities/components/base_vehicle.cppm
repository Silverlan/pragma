// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.base_vehicle;

export import :entities.components.base_physics;
export import :entities.components.base_wheel;

export {
	namespace pragma {
		namespace baseVehicleComponent {
			REGISTER_COMPONENT_EVENT(EVENT_ON_DRIVER_ENTERED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_DRIVER_EXITED)
		}
		class DLLNETWORK BaseVehicleComponent : public BaseEntityComponent {
		  public:
			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			enum class StateFlags : uint32_t { None = 0u, HasDriver = 1u, SteeringWheelInitialized = HasDriver << 1u };

			virtual void Initialize() override;
			virtual void OnRemove() override;
			ecs::BaseEntity *GetDriver();
			bool HasDriver() const;
			virtual void ClearDriver();
			virtual void SetDriver(ecs::BaseEntity *ent);
			uint8_t GetWheelCount() const;

			virtual void OnTick(double tDelta) override;

			void SetupVehicle(const physics::VehicleCreateInfo &createInfo, const std::vector<std::string> &wheelModels);

			ecs::BaseEntity *GetSteeringWheel();
			float GetSpeedKmh() const;
			float GetSteeringFactor() const;

			virtual void SetupSteeringWheel(const std::string &mdl, math::Degree maxSteeringAngle);

			physics::IVehicle *GetPhysicsVehicle();
			const physics::IVehicle *GetPhysicsVehicle() const;
		  protected:
			BaseVehicleComponent(ecs::BaseEntity &ent);
			struct DLLNETWORK WheelData {
				WheelData() = default;
				ComponentHandle<BaseWheelComponent> hWheel = {};
				std::string model = "";
			};
			NetEventId m_netEvSteeringWheelModel = INVALID_NET_EVENT;
			NetEventId m_netEvSetDriver = INVALID_NET_EVENT;
			std::vector<WheelData> m_wheels = {};
			physics::VehicleCreateInfo m_vhcCreateInfo = {};
			util::TSharedHandle<physics::IVehicle> m_physVehicle = nullptr;
			std::string m_steeringWheelMdl;
			EntityHandle m_steeringWheel = {};
			CallbackHandle m_cbSteeringWheel = {};
			EntityHandle m_driver = {};
			StateFlags m_stateFlags = StateFlags::None;
			float m_maxSteeringWheelAngle = 0.f;
			void InitializeVehiclePhysics(physics::PhysicsType type, BasePhysicsComponent::PhysFlags flags);
			void DestroyVehiclePhysics();
			virtual BaseWheelComponent *CreateWheelEntity(uint8_t wheelIndex);
			void InitializeWheelEntities();
			void InitializeSteeringWheel();
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseVehicleComponent::StateFlags)
};
