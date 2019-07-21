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
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		virtual void Initialize() override;
		virtual void OnRemove() override;
		BaseEntity *GetDriver();
		bool HasDriver() const;
		virtual void ClearDriver();
		virtual void SetDriver(BaseEntity *ent);
		uint8_t GetWheelCount() const;

		virtual void Think(double tDelta);

		void SetupVehicle(const pragma::physics::VehicleCreateInfo &createInfo,const std::vector<std::string> &wheelModels);

		BaseEntity *GetSteeringWheel();
		float GetSpeedKmh() const;
		float GetSteeringAngle() const;

		virtual void SetSteeringWheelModel(const std::string &mdl);

		physics::IVehicle *GetPhysicsVehicle();
		const physics::IVehicle *GetPhysicsVehicle() const;
	protected:
		BaseVehicleComponent(BaseEntity &ent);
		struct DLLNETWORK WheelData
		{
			WheelData()=default;
			util::WeakHandle<pragma::BaseWheelComponent> hWheel = {};
			std::string model = "";
		};
		pragma::NetEventId m_netEvSteeringWheelModel = pragma::INVALID_NET_EVENT;
		std::vector<WheelData> m_wheels = {};
		physics::VehicleCreateInfo m_vhcCreateInfo = {};
		util::TSharedHandle<physics::IVehicle> m_physVehicle = nullptr;
		std::string m_steeringWheelMdl;
		EntityHandle m_steeringWheel = {};
		CallbackHandle m_cbSteeringWheel = {};
		EntityHandle m_driver = {};
		bool m_bHasDriver = false;
		void InitializeVehiclePhysics(PHYSICSTYPE type,BasePhysicsComponent::PhysFlags flags);
		void DestroyVehiclePhysics();
		virtual BaseWheelComponent *CreateWheelEntity(uint8_t wheelIndex);
		void InitializeWheelEntities();
		void InitializeSteeringWheel();
	};
};

#endif
