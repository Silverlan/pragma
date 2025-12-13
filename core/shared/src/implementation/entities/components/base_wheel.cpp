// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

//#define ENABLE_DEPRECATED_PHYSICS

module pragma.shared;

import :entities.components.base_wheel;

using namespace pragma;

BaseWheelComponent::BaseWheelComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseWheelComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto &ent = GetEntity();
	ent.AddComponent("render");
	ent.AddComponent("transform");
	ent.AddComponent("model");
	SetTickPolicy(TickPolicy::Always); // TODO
	                                   /*m_netEvAttach = SetupNetEvent("attach");
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
	m_netEvMaxDampingRelaxation = SetupNetEvent("max_damping_relaxation");*/
}

BaseWheelComponent::~BaseWheelComponent()
{
	if(m_cbOnSpawn.IsValid())
		m_cbOnSpawn.Remove();
}
#if 0
void BaseWheelComponent::Attach(pragma::ecs::BaseEntity *ent,UChar wheelId)
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
			m_cbOnSpawn = pGenericComponentEnt->BindEventUnhandled(pragma::ecs::baseEntity::EVENT_ON_SPAWN,[this,fAttachWheel](std::reference_wrapper<pragma::ComponentEvent> evData) {
				auto cb = m_cbOnSpawn;
				volatile ScopeGuard sg([cb]() mutable {if(cb.IsValid()) cb.Remove();});
				if(GetEntity().IsSpawned() == true)
					fAttachWheel();
				else
				{
					auto *pGenericComponent = static_cast<pragma::BaseGenericComponent*>(GetEntity().FindComponent("entity").get());
					if(pGenericComponent != nullptr)
					{
						m_cbOnSpawn = pGenericComponent->BindEventUnhandled(pragma::ecs::baseEntity::EVENT_ON_SPAWN,[this,fAttachWheel](std::reference_wrapper<pragma::ComponentEvent> evData) {
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
			m_cbOnSpawn = pGenericComponent->BindEventUnhandled(pragma::ecs::baseEntity::EVENT_ON_SPAWN,[this,fAttachWheel](std::reference_wrapper<pragma::ComponentEvent> evData) {
				auto cb = m_cbOnSpawn;
				volatile ScopeGuard sg([cb]() mutable {if(cb.IsValid()) cb.Remove();});
				fAttachWheel();
			});
		}
	}
	else
		fAttachWheel();
	//

#ifdef ENABLE_DEPRECATED_PHYSICS
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
	info->m_steering = pragma::math::deg_to_rad(m_wheelInfo.steeringAngle);
	info->m_rotation = m_wheelInfo.wheelRotation;
	info->m_rollInfluence = m_wheelInfo.rollInfluence;
#endif
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
#endif
void BaseWheelComponent::UpdateWheel()
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	m_wheelInfo.steeringAngle = CFloat(pragma::math::rad_to_deg(info->m_steering));
	m_wheelInfo.wheelRotation = CFloat(info->m_rotation);
	auto &t = info->m_worldTransform;
	auto btOrigin = t.getOrigin() / PhysEnv::WORLD_SCALE;
	auto btRot = t.getRotation();

	auto origin = Vector3(btOrigin.x(), btOrigin.y(), btOrigin.z());
	origin += GetModelTranslation();
	auto rotation = Quat(CFloat(btRot.w()), CFloat(btRot.x()), CFloat(btRot.y()), CFloat(btRot.z()));
	rotation = rotation * GetModelRotation();
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent != nullptr) {
		pTrComponent->SetPosition(origin);
		pTrComponent->SetOrientation(rotation);
	}
#endif
}

void BaseWheelComponent::OnTick(double dt)
{
	UpdateWheel();
	UpdatePose();
}

void BaseWheelComponent::SetupWheel(BaseVehicleComponent &vhc, const physics::WheelCreateInfo &createInfo, uint8_t wheelId)
{
	m_createInfo = createInfo;
	m_vehicle = vhc.GetHandle<BaseVehicleComponent>();
	m_wheelId = wheelId;
}

void BaseWheelComponent::UpdatePose()
{
	if(m_vehicle.expired())
		return;
	auto *physVhc = m_vehicle->GetPhysicsVehicle();
	auto *body = physVhc ? physVhc->GetCollisionObject() : nullptr;
	if(physVhc == nullptr)
		return;
	auto pose = physVhc->GetLocalWheelPose(m_wheelId);
	if(pose.has_value() == false)
		return;
	auto &entVhc = m_vehicle->GetEntity();
	auto t = math::Transform {entVhc.GetPosition(), entVhc.GetRotation()};
	t *= *pose;

	auto &ent = GetEntity();
	ent.SetPosition(t.GetOrigin());
	ent.SetRotation(t.GetRotation());
}

#if 0
Bool BaseWheelComponent::IsAttached() const {return m_vehicle.valid();}
#ifdef ENABLE_DEPRECATED_PHYSICS
btWheelInfo *BaseWheelComponent::GetWheelInfo() const
{
	if(!IsAttached())
		return nullptr;
	return const_cast<BaseVehicleComponent*>(m_vehicle.get())->GetWheelInfo(m_wheelId);
}
#endif
pragma::util::WeakHandle<pragma::BaseVehicleComponent> BaseWheelComponent::GetVehicle() {return m_vehicle;}

Bool BaseWheelComponent::IsFrontWheel() const {return m_wheelInfo.bFrontWheel;}
void BaseWheelComponent::SetFrontWheel(Bool b)
{
	m_wheelInfo.bFrontWheel = b;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_bIsFrontWheel = b;
#endif
}
Vector3 &BaseWheelComponent::GetChassisConnectionPoint() {return m_wheelInfo.connectionPoint;}
void BaseWheelComponent::SetChassisConnectionPoint(const Vector3 &p)
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	m_wheelInfo.connectionPoint = p;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_chassisConnectionPointCS = btVector3(p.x,p.y,p.z) *PhysEnv::WORLD_SCALE;
#endif
}
Vector3 &BaseWheelComponent::GetWheelAxle() {return m_wheelInfo.wheelAxle;}
void BaseWheelComponent::SetWheelAxle(const Vector3 &axis)
{
	m_wheelInfo.wheelAxle = axis;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_wheelAxleCS = btVector3(axis.x,axis.y,axis.z);
#endif
}
Vector3 &BaseWheelComponent::GetWheelDirection() {return m_wheelInfo.wheelDirection;}
void BaseWheelComponent::SetWheelDirection(const Vector3 &dir)
{
	m_wheelInfo.wheelDirection = dir;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_wheelDirectionCS = btVector3(dir.x,dir.y,dir.z);
#endif
}
Float BaseWheelComponent::GetMaxSuspensionLength() const {return m_wheelInfo.suspensionLength;}
void BaseWheelComponent::SetMaxSuspensionLength(Float len)
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	m_wheelInfo.suspensionLength = len;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_suspensionRestLength1 = len *PhysEnv::WORLD_SCALE;
#endif
}
Float BaseWheelComponent::GetMaxSuspensionCompression() const {return m_wheelInfo.suspensionCompression;}
void BaseWheelComponent::SetMaxSuspensionCompression(Float cmp)
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	m_wheelInfo.suspensionCompression = cmp;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_maxSuspensionTravelCm = cmp *PhysEnv::WORLD_SCALE;
#endif
}
Float BaseWheelComponent::GetMaxDampingRelaxation() const {return m_wheelInfo.dampingRelaxation;}
void BaseWheelComponent::SetMaxDampingRelaxation(Float damping)
{
	m_wheelInfo.dampingRelaxation = damping;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_wheelsDampingRelaxation = damping;
#endif
}
Float BaseWheelComponent::GetWheelRadius() const {return m_wheelInfo.wheelRadius;}
void BaseWheelComponent::SetWheelRadius(Float radius)
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	m_wheelInfo.wheelRadius = radius;
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_wheelsRadius = radius *PhysEnv::WORLD_SCALE;
#endif
}
Float BaseWheelComponent::GetSuspensionStiffness() const {return m_wheelInfo.suspensionStiffness;}
void BaseWheelComponent::SetSuspensionStiffness(Float stiffness)
{
	m_wheelInfo.suspensionStiffness = stiffness;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_suspensionStiffness = stiffness;
#endif
}
Float BaseWheelComponent::GetWheelDampingCompression() const {return m_wheelInfo.wheelDampingCompression;}
void BaseWheelComponent::SetWheelDampingCompression(Float cmp)
{
	m_wheelInfo.wheelDampingCompression = cmp;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_wheelsDampingCompression = cmp;
#endif
}
Float BaseWheelComponent::GetFrictionSlip() const {return m_wheelInfo.frictionSlip;}
void BaseWheelComponent::SetFrictionSlip(Float slip)
{
	m_wheelInfo.frictionSlip = slip;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_frictionSlip = slip;
#endif
}
Float BaseWheelComponent::GetSteeringAngle() const {return m_wheelInfo.steeringAngle;}
void BaseWheelComponent::SetSteeringAngle(Float ang)
{
	m_wheelInfo.steeringAngle = ang;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_steering = pragma::math::deg_to_rad(ang);
#endif
}
Float BaseWheelComponent::GetWheelRotation() const {return m_wheelInfo.wheelRotation;}
void BaseWheelComponent::SetWheelRotation(Float rot)
{
	m_wheelInfo.wheelRotation = rot;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_rotation = rot;
#endif
}
Float BaseWheelComponent::GetRollInfluence() const {return m_wheelInfo.rollInfluence;}
void BaseWheelComponent::SetRollInfluence(Float influence)
{
	m_wheelInfo.rollInfluence = influence;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return;
	info->m_rollInfluence = influence;
#endif
}
Float BaseWheelComponent::GetRelativeSuspensionSpeed() const
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return 0.f;
	return static_cast<float>(info->m_suspensionRelativeVelocity /PhysEnv::WORLD_SCALE);
#else
	return 0.f;
#endif
}
Float BaseWheelComponent::GetSuspensionForce() const
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return 0.f;
	return static_cast<float>(info->m_wheelsSuspensionForce /PhysEnv::WORLD_SCALE);
#else
	return 0.f;
#endif
}
Float BaseWheelComponent::GetSkidGrip() const
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return 0.f;
	return static_cast<float>(info->m_skidInfo);
#else
	return 0.f;
#endif
}
Vector3 BaseWheelComponent::GetContactNormal() const
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return Vector3(0.f,0.f,0.f);
	auto &n = info->m_raycastInfo.m_contactNormalWS;
	return Vector3(n.x(),n.y(),n.z());
#else
	return Vector3{};
#endif
}
Vector3 BaseWheelComponent::GetContactPoint() const
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return Vector3(0.f,0.f,0.f);
	auto p = info->m_raycastInfo.m_contactPointWS /PhysEnv::WORLD_SCALE;
	return Vector3(p.x(),p.y(),p.z());
#else
	return Vector3{};
#endif
}
Bool BaseWheelComponent::IsInContact() const
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return false;
	return info->m_raycastInfo.m_isInContact;
#else
	return false;
#endif
}
Float BaseWheelComponent::GetSuspensionLength() const
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return 0.f;
	return static_cast<float>(info->m_raycastInfo.m_suspensionLength /PhysEnv::WORLD_SCALE);
#else
	return 0.f;
#endif
}
Vector3 BaseWheelComponent::GetWorldSpaceWheelAxle() const
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return Vector3(0.f,0.f,0.f);
	auto &axle = info->m_raycastInfo.m_wheelAxleWS;
	return Vector3(axle.x(),axle.y(),axle.z());
#else
	return Vector3{};
#endif
}
Vector3 BaseWheelComponent::GetWorldSpaceWheelDirection() const
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return Vector3(0.f,0.f,0.f);
	auto &dir = info->m_raycastInfo.m_wheelDirectionWS;
	return Vector3(dir.x(),dir.y(),dir.z());
#else
	return Vector3{};
#endif
}
pragma::physics::ICollisionObject *BaseWheelComponent::GetGroundObject() const
{
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo();
	if(info == nullptr)
		return nullptr;
	auto *o = static_cast<btCollisionObject*>(info->m_raycastInfo.m_groundObject);
	if(o == nullptr)
		return nullptr;
	return static_cast<pragma::physics::ICollisionObject*>(o->getUserPointer());
#else
	return nullptr;
#endif
}
#endif
