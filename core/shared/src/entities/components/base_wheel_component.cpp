#include "stdafx_shared.h"
#include "pragma/entities/components/base_wheel_component.hpp"
#include "pragma/physics/physenvironment.h"
#include "pragma/entities/components/base_vehicle_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_generic_component.hpp"
#include <sharedutils/scope_guard.h>

using namespace pragma;

WheelInfo::WheelInfo()
	: bFrontWheel(false),wheelAxle(1.f,0.f,0.f),
	wheelDirection(0.f,-1.f,0.f),suspensionLength(static_cast<float>(0.6f /PhysEnv::WORLD_SCALE)),
	suspensionCompression(static_cast<float>(1.f /PhysEnv::WORLD_SCALE)),wheelRadius(15.f),
	suspensionStiffness(50.f),wheelDampingCompression(0.2f),
	frictionSlip(0.8f),steeringAngle(0.f),wheelRotation(0.f),
	rollInfluence(1.f),connectionPoint(0.f,0.f,0.f),
	dampingRelaxation(0.3f)
{}

BaseWheelComponent::BaseWheelComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_wheelInfo(),m_wheelId(UChar(-1))
{}

void BaseWheelComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvAttach = SetupNetEvent("attach");
	m_netEvDetach = SetupNetEvent("detach");
	m_netEvFrontWheel = SetupNetEvent("front_wheel");
	m_netEvAxle = SetupNetEvent("axle");
	m_netEvDirection = SetupNetEvent("direction");
	m_netEvMaxSuspensionLength = SetupNetEvent("max_suspension_length");
	m_netEvMaxSuspensionCompression = SetupNetEvent("max_suspension_compression");
	m_netEvRadius = SetupNetEvent("radius");
	m_netEvSuspensionStiffness = SetupNetEvent("suspension_stiffness");
	m_netEvDampingCompression = SetupNetEvent("damping_compression");
	m_netEvFrictionSlip = SetupNetEvent("friction_slip");
	m_netEvSteeringAngle = SetupNetEvent("steering_angle");
	m_netEvRotation = SetupNetEvent("rotation");
	m_netEvRollInfluence = SetupNetEvent("roll_influence");
	m_netEvChassisConnectionPoint = SetupNetEvent("chassis_connection_point");
	m_netEvMaxDampingRelaxation = SetupNetEvent("max_damping_relaxation");
}

BaseWheelComponent::~BaseWheelComponent()
{
	if(m_cbOnSpawn.IsValid())
		m_cbOnSpawn.Remove();
}

void BaseWheelComponent::Attach(BaseEntity *ent,UChar wheelId)
{
	Detach();
	if(!ent->IsVehicle())
		return;
	m_vehicle = ent->GetVehicleComponent();
	m_wheelId = wheelId;

	// Attach the wheel to the vehicle, but make sure the wheel and the vehicle have been spawned!
	auto vhcComponent = ent->GetVehicleComponent();
	auto fAttachWheel = [ent,wheelId,this,&vhcComponent]() {vhcComponent->AttachWheel(wheelId,this);};
	if(!ent->IsSpawned())
	{
		auto *pGenericComponentEnt = static_cast<pragma::BaseGenericComponent*>(ent->FindComponent("entity").get());
		if(pGenericComponentEnt != nullptr)
		{
			m_cbOnSpawn = pGenericComponentEnt->BindEventUnhandled(BaseEntity::EVENT_ON_SPAWN,[this,fAttachWheel](std::reference_wrapper<pragma::ComponentEvent> evData) {
				auto cb = m_cbOnSpawn;
				volatile ScopeGuard sg([cb]() mutable {if(cb.IsValid()) cb.Remove();});
				if(GetEntity().IsSpawned() == true)
					fAttachWheel();
				else
				{
					auto *pGenericComponent = static_cast<pragma::BaseGenericComponent*>(GetEntity().FindComponent("entity").get());
					if(pGenericComponent != nullptr)
					{
						m_cbOnSpawn = pGenericComponent->BindEventUnhandled(BaseEntity::EVENT_ON_SPAWN,[this,fAttachWheel](std::reference_wrapper<pragma::ComponentEvent> evData) {
							auto cb = m_cbOnSpawn;
							volatile ScopeGuard sg([cb]() mutable {if(cb.IsValid()) cb.Remove();});
							fAttachWheel();
						});
					}
				}
			});
		}
	}
	else if(!GetEntity().IsSpawned())
	{
		auto *pGenericComponent = static_cast<pragma::BaseGenericComponent*>(GetEntity().FindComponent("entity").get());
		if(pGenericComponent != nullptr)
		{
			m_cbOnSpawn = pGenericComponent->BindEventUnhandled(BaseEntity::EVENT_ON_SPAWN,[this,fAttachWheel](std::reference_wrapper<pragma::ComponentEvent> evData) {
				auto cb = m_cbOnSpawn;
				volatile ScopeGuard sg([cb]() mutable {if(cb.IsValid()) cb.Remove();});
				fAttachWheel();
			});
		}
	}
	else
		fAttachWheel();
	//

	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_bIsFrontWheel = m_wheelInfo.bFrontWheel;
	info->m_chassisConnectionPointCS = btVector3(m_wheelInfo.connectionPoint.x,m_wheelInfo.connectionPoint.y,m_wheelInfo.connectionPoint.z) *PhysEnv::WORLD_SCALE;
	info->m_wheelAxleCS = btVector3(m_wheelInfo.wheelAxle.x,m_wheelInfo.wheelAxle.y,m_wheelInfo.wheelAxle.z);
	info->m_wheelDirectionCS = btVector3(m_wheelInfo.wheelDirection.x,m_wheelInfo.wheelDirection.y,m_wheelInfo.wheelDirection.z);
	info->m_suspensionRestLength1 = m_wheelInfo.suspensionLength *PhysEnv::WORLD_SCALE;
	info->m_maxSuspensionTravelCm = m_wheelInfo.suspensionCompression *PhysEnv::WORLD_SCALE;
	info->m_wheelsRadius = m_wheelInfo.wheelRadius *PhysEnv::WORLD_SCALE;
	info->m_suspensionStiffness = m_wheelInfo.suspensionStiffness;
	info->m_wheelsDampingCompression = m_wheelInfo.wheelDampingCompression;
	info->m_frictionSlip = m_wheelInfo.frictionSlip;
	info->m_steering = umath::deg_to_rad(m_wheelInfo.steeringAngle);
	info->m_rotation = m_wheelInfo.wheelRotation;
	info->m_rollInfluence = m_wheelInfo.rollInfluence;
}

void BaseWheelComponent::Detach()
{
	m_wheelId = UChar(-1);
	if(m_vehicle.expired())
		return;
	auto &vhcComponent = *m_vehicle;
	m_vehicle = {};
	vhcComponent.DetachWheel(m_wheelId);
}

void BaseWheelComponent::SetModelTranslation(const Vector3 &v) {m_modelTranslation = v;}
Vector3 &BaseWheelComponent::GetModelTranslation() {return m_modelTranslation;}
void BaseWheelComponent::SetModelRotation(const Quat &rot) {m_modelRotation = rot;}
Quat &BaseWheelComponent::GetModelRotation() {return m_modelRotation;}

void BaseWheelComponent::UpdateWheel()
{
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	m_wheelInfo.steeringAngle = CFloat(umath::rad_to_deg(info->m_steering));
	m_wheelInfo.wheelRotation = CFloat(info->m_rotation);
	auto &t = info->m_worldTransform;
	auto btOrigin = t.getOrigin() /PhysEnv::WORLD_SCALE;
	auto btRot = t.getRotation();

	auto origin = Vector3(btOrigin.x(),btOrigin.y(),btOrigin.z());
	origin += GetModelTranslation();
	auto rotation = Quat(CFloat(btRot.w()),CFloat(btRot.x()),CFloat(btRot.y()),CFloat(btRot.z()));
	rotation = rotation *GetModelRotation();
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.valid())
	{
		pTrComponent->SetPosition(origin);
		pTrComponent->SetOrientation(rotation);
	}
}

void BaseWheelComponent::Think(double)
{
	UpdateWheel();
}

Bool BaseWheelComponent::IsAttached() const {return m_vehicle.valid();}

btWheelInfo *BaseWheelComponent::GetWheelInfo() const
{
	if(!IsAttached())
		return nullptr;
	return const_cast<BaseVehicleComponent*>(m_vehicle.get())->GetWheelInfo(m_wheelId);
}

util::WeakHandle<pragma::BaseVehicleComponent> BaseWheelComponent::GetVehicle() {return m_vehicle;}

Bool BaseWheelComponent::IsFrontWheel() const {return m_wheelInfo.bFrontWheel;}
void BaseWheelComponent::SetFrontWheel(Bool b)
{
	m_wheelInfo.bFrontWheel = b;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_bIsFrontWheel = b;
}
Vector3 &BaseWheelComponent::GetChassisConnectionPoint() {return m_wheelInfo.connectionPoint;}
void BaseWheelComponent::SetChassisConnectionPoint(const Vector3 &p)
{
	m_wheelInfo.connectionPoint = p;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_chassisConnectionPointCS = btVector3(p.x,p.y,p.z) *PhysEnv::WORLD_SCALE;
}
Vector3 &BaseWheelComponent::GetWheelAxle() {return m_wheelInfo.wheelAxle;}
void BaseWheelComponent::SetWheelAxle(const Vector3 &axis)
{
	m_wheelInfo.wheelAxle = axis;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_wheelAxleCS = btVector3(axis.x,axis.y,axis.z);
}
Vector3 &BaseWheelComponent::GetWheelDirection() {return m_wheelInfo.wheelDirection;}
void BaseWheelComponent::SetWheelDirection(const Vector3 &dir)
{
	m_wheelInfo.wheelDirection = dir;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_wheelDirectionCS = btVector3(dir.x,dir.y,dir.z);
}
Float BaseWheelComponent::GetMaxSuspensionLength() const {return m_wheelInfo.suspensionLength;}
void BaseWheelComponent::SetMaxSuspensionLength(Float len)
{
	m_wheelInfo.suspensionLength = len;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_suspensionRestLength1 = len *PhysEnv::WORLD_SCALE;
}
Float BaseWheelComponent::GetMaxSuspensionCompression() const {return m_wheelInfo.suspensionCompression;}
void BaseWheelComponent::SetMaxSuspensionCompression(Float cmp)
{
	m_wheelInfo.suspensionCompression = cmp;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_maxSuspensionTravelCm = cmp *PhysEnv::WORLD_SCALE;
}
Float BaseWheelComponent::GetMaxDampingRelaxation() const {return m_wheelInfo.dampingRelaxation;}
void BaseWheelComponent::SetMaxDampingRelaxation(Float damping)
{
	m_wheelInfo.dampingRelaxation = damping;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_wheelsDampingRelaxation = damping;
}
Float BaseWheelComponent::GetWheelRadius() const {return m_wheelInfo.wheelRadius;}
void BaseWheelComponent::SetWheelRadius(Float radius)
{
	m_wheelInfo.wheelRadius = radius;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_wheelsRadius = radius *PhysEnv::WORLD_SCALE;
}
Float BaseWheelComponent::GetSuspensionStiffness() const {return m_wheelInfo.suspensionStiffness;}
void BaseWheelComponent::SetSuspensionStiffness(Float stiffness)
{
	m_wheelInfo.suspensionStiffness = stiffness;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_suspensionStiffness = stiffness;
}
Float BaseWheelComponent::GetWheelDampingCompression() const {return m_wheelInfo.wheelDampingCompression;}
void BaseWheelComponent::SetWheelDampingCompression(Float cmp)
{
	m_wheelInfo.wheelDampingCompression = cmp;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_wheelsDampingCompression = cmp;
}
Float BaseWheelComponent::GetFrictionSlip() const {return m_wheelInfo.frictionSlip;}
void BaseWheelComponent::SetFrictionSlip(Float slip)
{
	m_wheelInfo.frictionSlip = slip;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_frictionSlip = slip;
}
Float BaseWheelComponent::GetSteeringAngle() const {return m_wheelInfo.steeringAngle;}
void BaseWheelComponent::SetSteeringAngle(Float ang)
{
	m_wheelInfo.steeringAngle = ang;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_steering = umath::deg_to_rad(ang);
}
Float BaseWheelComponent::GetWheelRotation() const {return m_wheelInfo.wheelRotation;}
void BaseWheelComponent::SetWheelRotation(Float rot)
{
	m_wheelInfo.wheelRotation = rot;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_rotation = rot;
}
Float BaseWheelComponent::GetRollInfluence() const {return m_wheelInfo.rollInfluence;}
void BaseWheelComponent::SetRollInfluence(Float influence)
{
	m_wheelInfo.rollInfluence = influence;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_rollInfluence = influence;
}
Float BaseWheelComponent::GetRelativeSuspensionSpeed() const
{
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return 0.f;
	return static_cast<float>(info->m_suspensionRelativeVelocity /PhysEnv::WORLD_SCALE);
}
Float BaseWheelComponent::GetSuspensionForce() const
{
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return 0.f;
	return static_cast<float>(info->m_wheelsSuspensionForce /PhysEnv::WORLD_SCALE);
}
Float BaseWheelComponent::GetSkidGrip() const
{
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return 0.f;
	return static_cast<float>(info->m_skidInfo);
}
Vector3 BaseWheelComponent::GetContactNormal() const
{
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return Vector3(0.f,0.f,0.f);
	auto &n = info->m_raycastInfo.m_contactNormalWS;
	return Vector3(n.x(),n.y(),n.z());
}
Vector3 BaseWheelComponent::GetContactPoint() const
{
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return Vector3(0.f,0.f,0.f);
	auto p = info->m_raycastInfo.m_contactPointWS /PhysEnv::WORLD_SCALE;
	return Vector3(p.x(),p.y(),p.z());
}
Bool BaseWheelComponent::IsInContact() const
{
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return false;
	return info->m_raycastInfo.m_isInContact;
}
Float BaseWheelComponent::GetSuspensionLength() const
{
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return 0.f;
	return static_cast<float>(info->m_raycastInfo.m_suspensionLength /PhysEnv::WORLD_SCALE);
}
Vector3 BaseWheelComponent::GetWorldSpaceWheelAxle() const
{
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return Vector3(0.f,0.f,0.f);
	auto &axle = info->m_raycastInfo.m_wheelAxleWS;
	return Vector3(axle.x(),axle.y(),axle.z());
}
Vector3 BaseWheelComponent::GetWorldSpaceWheelDirection() const
{
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return Vector3(0.f,0.f,0.f);
	auto &dir = info->m_raycastInfo.m_wheelDirectionWS;
	return Vector3(dir.x(),dir.y(),dir.z());
}
PhysCollisionObject *BaseWheelComponent::GetGroundObject() const
{
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return nullptr;
	auto *o = static_cast<btCollisionObject*>(info->m_raycastInfo.m_groundObject);
	if(o == nullptr)
		return nullptr;
	return static_cast<PhysCollisionObject*>(o->getUserPointer());
}