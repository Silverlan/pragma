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
		virtual void Initialize() override;
		virtual void OnRemove() override;
		BaseEntity *GetDriver();
		bool HasDriver() const;
		virtual void ClearDriver();
		virtual void SetDriver(BaseEntity *ent);
		uint8_t GetWheelCount() const;

		virtual void Think(double tDelta);

		// TODO: Allow different models per wheel
		void SetupVehicle(const pragma::physics::VehicleCreateInfo &createInfo,const std::string &wheelModel);

		BaseEntity *GetSteeringWheel();
		float GetSpeedKmh() const;
		float GetSteeringAngle() const;

		virtual void SetSteeringWheelModel(const std::string &mdl);

		bool IsFirstPersonCameraEnabled() const;
		bool IsThirdPersonCameraEnabled() const;
		virtual void SetFirstPersonCameraEnabled(bool b);
		virtual void SetThirdPersonCameraEnabled(bool b);

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
		std::vector<WheelData> m_wheels = {};
		physics::VehicleCreateInfo m_vhcCreateInfo = {};
		util::TSharedHandle<physics::IVehicle> m_physVehicle = nullptr;
		EntityHandle m_steeringWheel = {};
		CallbackHandle m_cbSteeringWheel = {};
		EntityHandle m_driver = {};
		bool m_bFirstPersonCameraEnabled = true;
		bool m_bThirdPersonCameraEnabled = true;
		void InitializeVehiclePhysics(PHYSICSTYPE type,BasePhysicsComponent::PhysFlags flags);
		void DestroyVehiclePhysics();
		virtual Bool AddWheel(const Vector3 &connectionPoint,const Vector3 &wheelAxle,Bool bIsFrontWheel,UChar *wheelId,const Vector3 &mdlOffset={},const Quat &mdlRotOffset={});
		BaseWheelComponent *CreateWheelEntity(uint8_t wheelIndex);
		void InitializeWheelEntities();
		void InitializeSteeringWheel();
	};
};

#endif
