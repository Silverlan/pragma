#ifndef __BASEWHEEL_H__
#define __BASEWHEEL_H__

#include "pragma/networkdefinitions.h"

struct DLLNETWORK WheelInfo
{
	WheelInfo();
	Bool bFrontWheel;
	Vector3 connectionPoint;
	Vector3 wheelAxle;
	Vector3 wheelDirection;
	Float suspensionLength;
	Float suspensionCompression;
	Float dampingRelaxation;
	Float wheelRadius;
	Float suspensionStiffness;
	Float wheelDampingCompression;
	Float frictionSlip;
	Float steeringAngle;
	Float wheelRotation;
	Float rollInfluence;
};

namespace pragma
{
	class BaseVehicleComponent;
	class DLLNETWORK BaseWheelComponent
		: public BaseEntityComponent
	{
	private:
		CallbackHandle m_cbOnSpawn = {};
	protected:
		BaseWheelComponent(BaseEntity &ent);
		util::WeakHandle<pragma::BaseVehicleComponent> m_vehicle = {};
		UChar m_wheelId = 0u;
		WheelInfo m_wheelInfo = {};
		Vector3 m_modelTranslation = {};
		Quat m_modelRotation = uquat::identity();

		pragma::NetEventId m_netEvAttach = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvDetach = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvFrontWheel = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvAxle = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvDirection = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvMaxSuspensionLength = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvMaxSuspensionCompression = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvRadius = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSuspensionStiffness = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvDampingCompression = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvFrictionSlip = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSteeringAngle = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvRotation = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvRollInfluence = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvChassisConnectionPoint = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvMaxDampingRelaxation = pragma::INVALID_NET_EVENT;

		btWheelInfo *GetWheelInfo() const;
		virtual void Think(double tDelta);
		void UpdateWheel();
	public:
		virtual ~BaseWheelComponent() override;
		virtual void Initialize() override;
		virtual void Attach(BaseEntity *ent,UChar wheelId);
		virtual void Detach();
		util::WeakHandle<pragma::BaseVehicleComponent> GetVehicle();
		Bool IsAttached() const;
		Bool IsFrontWheel() const;
		void SetModelTranslation(const Vector3 &v);
		Vector3 &GetModelTranslation();
		void SetModelRotation(const Quat &rot);
		Quat &GetModelRotation();
		virtual void SetFrontWheel(Bool b);
		Vector3 &GetChassisConnectionPoint();
		virtual void SetChassisConnectionPoint(const Vector3 &p);
		Vector3 &GetWheelAxle();
		virtual void SetWheelAxle(const Vector3 &axis);
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
		Float GetWheelRotation() const;
		virtual void SetWheelRotation(Float rot);
		Float GetRollInfluence() const;
		virtual void SetRollInfluence(Float influence);
		Float GetRelativeSuspensionSpeed() const;
		Float GetSuspensionForce() const;
		Float GetSkidGrip() const;
		Vector3 GetContactNormal() const;
		Vector3 GetContactPoint() const;
		Bool IsInContact() const;
		Float GetSuspensionLength() const;
		Vector3 GetWorldSpaceWheelAxle() const;
		Vector3 GetWorldSpaceWheelDirection() const;
		PhysCollisionObject *GetGroundObject() const;
	};
};

#endif
