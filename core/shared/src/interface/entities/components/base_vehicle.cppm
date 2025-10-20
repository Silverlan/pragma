// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.base_vehicle;

export import :entities.components.base_physics;
export import :entities.components.base_wheel;

export {
	namespace pragma {
		class DLLNETWORK BaseVehicleComponent : public BaseEntityComponent {
		public:
			static ComponentEventId EVENT_ON_DRIVER_ENTERED;
			static ComponentEventId EVENT_ON_DRIVER_EXITED;
			static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			enum class StateFlags : uint32_t { None = 0u, HasDriver = 1u, SteeringWheelInitialized = HasDriver << 1u };

			virtual void Initialize() override;
			virtual void OnRemove() override;
			pragma::ecs::BaseEntity *GetDriver();
			bool HasDriver() const;
			virtual void ClearDriver();
			virtual void SetDriver(pragma::ecs::BaseEntity *ent);
			uint8_t GetWheelCount() const;

			virtual void OnTick(double tDelta) override;

			void SetupVehicle(const pragma::physics::VehicleCreateInfo &createInfo, const std::vector<std::string> &wheelModels);

			pragma::ecs::BaseEntity *GetSteeringWheel();
			float GetSpeedKmh() const;
			float GetSteeringFactor() const;

			virtual void SetupSteeringWheel(const std::string &mdl, umath::Degree maxSteeringAngle);

			physics::IVehicle *GetPhysicsVehicle();
			const physics::IVehicle *GetPhysicsVehicle() const;
		protected:
			BaseVehicleComponent(pragma::ecs::BaseEntity &ent);
			struct DLLNETWORK WheelData {
				WheelData() = default;
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
			void InitializeVehiclePhysics(pragma::physics::PHYSICSTYPE type, BasePhysicsComponent::PhysFlags flags);
			void DestroyVehiclePhysics();
			virtual BaseWheelComponent *CreateWheelEntity(uint8_t wheelIndex);
			void InitializeWheelEntities();
			void InitializeSteeringWheel();
		};
        using namespace umath::scoped_enum::bitwise;
	};
    namespace umath::scoped_enum::bitwise {
        template<>
        struct enable_bitwise_operators<pragma::BaseVehicleComponent::StateFlags> : std::true_type {};
    }
};
