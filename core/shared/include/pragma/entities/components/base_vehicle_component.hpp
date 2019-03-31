#ifndef __BASE_VEHICLE_COMPONENT_HPP__
#define __BASE_VEHICLE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/components/base_wheel_component.hpp"
#ifdef __linux__
#include "pragma/basevehicle_raycaster.h"
#endif

#define NET_EVENT_VEHICLE_ADD_WHEEL 50001
#define NET_EVENT_VEHICLE_SET_DRIVER 50002
#define NET_EVENT_VEHICLE_MAX_ENGINE_FORCE 50003
#define NET_EVENT_VEHICLE_MAX_REVERSE_ENGINE_FORCE 50004
#define NET_EVENT_VEHICLE_MAX_BRAKE_FORCE 50005
#define NET_EVENT_VEHICLE_ACCELERATION 50006
#define NET_EVENT_VEHICLE_TURN_SPEED 50007
#define NET_EVENT_VEHICLE_MAX_TURN_ANGLE 50008
#define NET_EVENT_VEHICLE_SET_STEERING_WHEEL 50011
#define NET_EVENT_VEHICLE_SET_FIRST_PERSON_CAMERA_ENABLED 50012
#define NET_EVENT_VEHICLE_SET_THIRD_PERSON_CAMERA_ENABLED 50013

struct PhysVehicleRaycaster;
namespace pragma
{
	class DLLNETWORK BaseVehicleComponent
		: public BaseEntityComponent
	{
	public:
		virtual ~BaseVehicleComponent() override;
		virtual void Initialize() override;
		BaseEntity *GetDriver();
		Bool HasDriver() const;
		virtual void ClearDriver();
		virtual void SetDriver(BaseEntity *ent);
		unsigned char GetWheelCount();
		btWheelInfo *GetWheelInfo(int wheel);
		btRaycastVehicle *GetBtVehicle();
		virtual void Think(double tDelta);
		void DetachWheel(UChar wheelId);
		void AttachWheel(UChar wheelId,pragma::BaseWheelComponent *wheel);
		std::vector<util::WeakHandle<pragma::BaseWheelComponent>> GetWheels() const;
		util::WeakHandle<pragma::BaseWheelComponent> GetWheel(UChar wheelId);
		BaseEntity *GetSteeringWheel();
		Float GetSpeedKmh() const;
		Float GetEngineForce() const;
		Float GetBrakeForce() const;
		Float GetMaxEngineForce() const;
		Float GetMaxReverseEngineForce() const;
		Float GetMaxBrakeForce() const;
		Float GetAcceleration() const;
		Float GetTurnSpeed() const;
		Float GetMaxTurnAngle() const;
		void SetEngineForce(Float force);
		void SetBrakeForce(Float force);
		virtual void SetMaxEngineForce(Float force);
		virtual void SetMaxReverseEngineForce(Float force);
		virtual void SetMaxBrakeForce(Float force);
		virtual void SetAcceleration(Float acc);
		virtual void SetTurnSpeed(Float speed);
		virtual void SetMaxTurnAngle(Float ang);
		virtual void SetSteeringWheelModel(const std::string &mdl);
		bool IsFirstPersonCameraEnabled() const;
		bool IsThirdPersonCameraEnabled() const;
		virtual void SetFirstPersonCameraEnabled(bool b);
		virtual void SetThirdPersonCameraEnabled(bool b);

		// Wheel Info
		Vector3 &GetWheelDirection();
		virtual void SetWheelDirection(const Vector3 &dir);
		Float GetMaxSuspensionLength() const;
		virtual void SetMaxSuspensionLength(Float len);
		Float GetMaxSuspensionCompression() const;
		virtual void SetMaxSuspensionCompression(Float cmp);
		Float GetMaxDampingRelaxation() const;
		virtual void SetMaxDampingRelaxation(Float damping);
		Float GetWheelRadius() const;
		virtual void SetWheelRadius(Float radius);
		Float GetSuspensionStiffness() const;
		virtual void SetSuspensionStiffness(Float stiffness);
		Float GetWheelDampingCompression() const;
		virtual void SetWheelDampingCompression(Float cmp);
		Float GetFrictionSlip() const;
		virtual void SetFrictionSlip(Float slip);
		Float GetSteeringAngle() const;
		virtual void SetSteeringAngle(Float ang);
		Float GetRollInfluence() const;
		virtual void SetRollInfluence(Float influence);
		//
	protected:
		BaseVehicleComponent(BaseEntity &ent);
		struct DLLNETWORK WheelData
		{
			WheelData();
			util::WeakHandle<pragma::BaseWheelComponent> hWheel = {};
			Vector3 connectionPoint = {};
			Vector3 wheelAxle = {};
			Bool bIsFrontWheel = false;
			Vector3 modelTranslation = {};
			Quat modelRotation = uquat::identity();
		};
		std::vector<WheelData> m_wheels;
		EntityHandle m_steeringWheel = {};
		std::shared_ptr<PhysConvexShape> m_shape = nullptr;
		std::unique_ptr<PhysRigidBody> m_rigidBody = nullptr;
		btRaycastVehicle::btVehicleTuning m_tuning = {};
		std::unique_ptr<PhysVehicleRaycaster> m_vhcRayCaster = nullptr;
		std::unique_ptr<btRaycastVehicle> m_vhcRaycast = nullptr;
		WheelInfo m_wheelInfo = {};
		CallbackHandle m_cbSteeringWheel = {};
		EntityHandle m_driver = {};
		Float m_engineForce = 0.f;
		Float m_brakeForce = 0.f;
		Float m_maxEngineForce = 0.f;
		Float m_maxReverseEngineForce = 0.f;
		Float m_maxBrakeForce = 0.f;
		Float m_acceleration = 0.f;
		Float m_turnSpeed = 0.f;
		Float m_maxTurnAngle = 0.f;
		bool m_bFirstPersonCameraEnabled = false;
		bool m_bThirdPersonCameraEnabled = false;
		virtual void OnPhysicsInitialized();
		virtual void OnPhysicsDestroyed();
		virtual Bool AddWheel(const Vector3 &connectionPoint,const Vector3 &wheelAxle,Bool bIsFrontWheel,UChar *wheelId,const Vector3 &mdlOffset={},const Quat &mdlRotOffset={});
		void InitializeWheelEntity(pragma::BaseWheelComponent *wheel,const WheelData &data);
		void InitializeSteeringWheel();

		void InitializeWheel(const WheelData &data);
		void InitializeWheels();
	};
};

#endif
