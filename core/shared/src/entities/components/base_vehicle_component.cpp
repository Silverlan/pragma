#include "stdafx_shared.h"
#include "pragma/entities/components/base_vehicle_component.hpp"
#include "pragma/basevehicle_raycaster.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/input/inkeys.h"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_attachable_component.hpp"
#include "pragma/physics/raytraces.h"

using namespace pragma;

PhysVehicleRaycaster::PhysVehicleRaycaster(Game *game,pragma::BaseVehicleComponent &vhc)
	: m_game(game),m_vehicle(vhc),m_entity(vhc.GetEntity())
{
#if USE_CONVEX_WHEELS == 1
	auto radius = vhc->GetWheelRadius();
	//m_shape = m_game->GetPhysicsEnvironment()->CreateCylinderShape(radius,radius *0.5f);
	m_shape = m_game->GetPhysicsEnvironment()->CreateSphereShape(radius);
#endif
}
PhysVehicleRaycaster::~PhysVehicleRaycaster()
{}
void *PhysVehicleRaycaster::castRay(const btVector3 &from,const btVector3 &to, btVehicleRaycasterResult &result)
{
#if USE_CONVEX_WHEELS == 1
	const auto axisRot = static_cast<float>(umath::sin(umath::deg_to_rad(45.0)));
	const auto shapeRot = uquat::create(EulerAngles(0.f,0.f,90.f));//Quat{axisRot,0.f,0.f,axisRot}; // 90 degree rotation
#endif
	auto start = Vector3(from.x(),from.y(),from.z()) /static_cast<float>(PhysEnv::WORLD_SCALE);
	auto end = Vector3(to.x(),to.y(),to.z()) /static_cast<float>(PhysEnv::WORLD_SCALE);
	TraceData trData;
	trData.SetSource(start);
	trData.SetTarget(end);
	trData.SetCollisionFilterMask(CollisionMask::All &~CollisionMask::Vehicle);
#if USE_CONVEX_WHEELS == 1
	//trData.SetFilter(dynamic_cast<BaseEntity*>(m_game->GetWorld())->GetHandle());
	trData.SetSourceRotation(m_entity->GetOrientation() *shapeRot);
	trData.SetSource(m_shape->GetConvexShape());
	auto r = m_game->Sweep(trData);
#else
	auto r = m_game->RayCast(trData);
#endif
	if(r.hit == true && r.collisionObj != nullptr)
	{
		auto *body = btRigidBody::upcast(r.collisionObj->GetCollisionObject());
		if(body != nullptr && body->hasContactResponse())
		{
#if USE_CONVEX_WHEELS == 1
			auto hitPos = r.position +r.normal *19.5f; // TODO
			auto d = uvec::distance(start,end);
			auto fraction = (r.fraction *d) /(d +19.5f);
#else
			auto &hitPos = r.position;
			auto fraction = r.fraction;
#endif
			result.m_hitPointInWorld = btVector3(hitPos.x,hitPos.y,hitPos.z) *PhysEnv::WORLD_SCALE;
			result.m_hitNormalInWorld = btVector3(r.normal.x,r.normal.y,r.normal.z);
			result.m_hitNormalInWorld.normalize();
			result.m_distFraction = fraction;
			return (void*)body;
		}
	}
	return 0;
}

BaseVehicleComponent::WheelData::WheelData()
	: hWheel()
{}

#ifdef PHYS_ENGINE_BULLET
void BaseVehicleComponent::OnPhysicsInitialized()
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr || !phys->IsRigid())
		return;
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	auto *physWorld = physEnv->GetWorld();
	auto *physRigid = static_cast<RigidPhysObj*>(phys);

	for(auto &o : physRigid->GetCollisionObjects())
		o->SetContactProcessingThreshold(0.f); // Without this vehicles stumble over edges between meshes frequently

	auto *rigidBody = physRigid->GetRigidBody();
	if(rigidBody == nullptr)
		return;
	auto *btRigidBody = rigidBody->GetRigidBody();
	m_vhcRayCaster = std::make_unique<PhysVehicleRaycaster>(game,*this);
	rigidBody->SetActivationState(DISABLE_DEACTIVATION);
	m_vhcRaycast = std::make_unique<btRaycastVehicle>(m_tuning,btRigidBody,m_vhcRayCaster.get());
	m_vhcRaycast->setCoordinateSystem(0,1,2);
	physWorld->addVehicle(m_vhcRaycast.get());

	InitializeWheels();
}

void BaseVehicleComponent::OnPhysicsDestroyed()
{
	if(m_vhcRaycast != nullptr)
	{
		auto &ent = GetEntity();
		auto *nw = ent.GetNetworkState();
		auto *game = nw->GetGameState();
		auto *physEnv = game->GetPhysicsEnvironment();
		auto *physWorld = physEnv->GetWorld();
		physWorld->removeVehicle(m_vhcRaycast.get());
	}
	m_vhcRaycast = nullptr;
	m_vhcRayCaster = nullptr;
}
#elif PHYS_ENGINE_PHYSX
void BaseVehicleComponent::OnSpawn()
{
	unsigned char numWheels = GetWheelCount();
}
#endif

BaseVehicleComponent::BaseVehicleComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_wheelInfo(),m_engineForce(0.f),
	m_brakeForce(0.f),m_maxEngineForce(100.f),
	m_maxReverseEngineForce(80.f),m_maxBrakeForce(200.f),
	m_acceleration(200.f),m_turnSpeed(90.f),m_maxTurnAngle(45.f),
	m_bFirstPersonCameraEnabled(true),m_bThirdPersonCameraEnabled(true)
{}

BaseVehicleComponent::~BaseVehicleComponent()
{
	if(m_cbSteeringWheel.IsValid())
		m_cbSteeringWheel.Remove();
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	auto *physWorld = physEnv->GetWorld();
	if(m_vhcRaycast != nullptr)
	{
		auto *vhc = m_vhcRaycast.get();
		physWorld->removeVehicle(vhc);
	}
}

bool BaseVehicleComponent::IsFirstPersonCameraEnabled() const {return m_bFirstPersonCameraEnabled;}
bool BaseVehicleComponent::IsThirdPersonCameraEnabled() const {return m_bThirdPersonCameraEnabled;}
void BaseVehicleComponent::SetFirstPersonCameraEnabled(bool b) {m_bFirstPersonCameraEnabled = b;}
void BaseVehicleComponent::SetThirdPersonCameraEnabled(bool b) {m_bThirdPersonCameraEnabled = b;}

Float BaseVehicleComponent::GetAcceleration() const {return m_acceleration;}
Float BaseVehicleComponent::GetTurnSpeed() const {return m_turnSpeed;}
Float BaseVehicleComponent::GetEngineForce() const {return m_engineForce;}
Float BaseVehicleComponent::GetBrakeForce() const {return m_brakeForce;}
Float BaseVehicleComponent::GetMaxTurnAngle() const {return m_maxTurnAngle;}
Float BaseVehicleComponent::GetSpeedKmh() const
{
	if(m_vhcRaycast == nullptr)
		return 0.f;
	return CFloat(m_vhcRaycast->getCurrentSpeedKmHour());
}

std::vector<util::WeakHandle<pragma::BaseWheelComponent>> BaseVehicleComponent::GetWheels() const
{
	std::vector<util::WeakHandle<pragma::BaseWheelComponent>> wheels;
	wheels.reserve(m_wheels.size());
	for(auto &data : m_wheels)
		wheels.push_back(data.hWheel);
	return wheels;
}
util::WeakHandle<pragma::BaseWheelComponent> BaseVehicleComponent::GetWheel(UChar wheelId)
{
	if(wheelId >= m_wheels.size())
		return nullptr;
	return m_wheels[wheelId].hWheel;
}

void BaseVehicleComponent::AttachWheel(UChar wheelId,pragma::BaseWheelComponent *wheel)
{
	auto numWheels = GetWheelCount();
	assert(wheelId < numWheels);
	if(wheelId >= numWheels)
		return;
	if(m_wheels.size() < numWheels)
		m_wheels.resize(numWheels);
	m_wheels[wheelId].hWheel = wheel->GetHandle<pragma::BaseWheelComponent>();
	InitializeWheelEntity(wheel,m_wheels[wheelId]);
}

btRaycastVehicle *BaseVehicleComponent::GetBtVehicle() {return (m_vhcRaycast != nullptr) ? m_vhcRaycast.get() : nullptr;}

void BaseVehicleComponent::DetachWheel(UChar wheelId)
{
	auto numWheels = GetWheelCount();
	if(wheelId >= numWheels)
		return;
	if(wheelId < m_wheels.size())
	{
		auto &data = m_wheels[wheelId];
		if(data.hWheel.valid())
			data.hWheel->Detach();
		m_wheels.erase(m_wheels.begin() +wheelId);
	}
	// TODO Detach bullet wheel
}

void BaseVehicleComponent::InitializeSteeringWheel()
{
	auto *ent = GetSteeringWheel();
	if(ent == nullptr)
		return;
	if(m_cbSteeringWheel.IsValid())
		m_cbSteeringWheel.Remove();
	auto *pAttComponent = static_cast<BaseAttachableComponent*>(ent->FindComponent("attachable").get());
	if(pAttComponent != nullptr)
	{
		m_cbSteeringWheel = pAttComponent->BindEventUnhandled(BaseAttachableComponent::EVENT_ON_ATTACHMENT_UPDATE,[this,pAttComponent](std::reference_wrapper<pragma::ComponentEvent> evData) {
			auto pTrComponentSteeringWheel = pAttComponent->GetEntity().GetTransformComponent();
			if(pTrComponentSteeringWheel.expired())
				return;
			auto ang = EulerAngles(-GetSteeringAngle(),0.f,0.f);
			auto rot = uquat::create(ang);
			auto rotEnt = pTrComponentSteeringWheel->GetOrientation();
			rotEnt = rotEnt *rot;
			pTrComponentSteeringWheel->SetOrientation(rotEnt);
		});
	}
}

void BaseVehicleComponent::SetSteeringWheelModel(const std::string &mdl)
{
	if(!m_steeringWheel.IsValid())
		return;
	auto mdlComponent = m_steeringWheel->GetModelComponent();
	if(mdlComponent.expired())
		return;
	mdlComponent->SetModel(mdl.c_str());
}

void BaseVehicleComponent::ClearDriver()
{
	if(!m_driver.IsValid())
		return;
	m_driver = EntityHandle();
}

void BaseVehicleComponent::SetDriver(BaseEntity *ent)
{
	if(m_driver.IsValid())
		ClearDriver();
	m_driver = ent->GetHandle();
}

BaseEntity *BaseVehicleComponent::GetDriver() {return m_driver.get();}
Bool BaseVehicleComponent::HasDriver() const {return m_driver.IsValid();}

void BaseVehicleComponent::InitializeWheel(const WheelData &data)
{
	if(m_vhcRaycast == nullptr)
		return;
	auto &connectionPoint = data.connectionPoint;
	auto &wheelAxle = data.wheelAxle;
	auto bIsFrontWheel = data.bIsFrontWheel;

	auto &wheelDir = GetWheelDirection();
	auto suspensionRestLength = GetMaxSuspensionLength() *PhysEnv::WORLD_SCALE;
	auto wheelRadius = GetWheelRadius();
	auto &info = m_vhcRaycast->addWheel(
		btVector3(connectionPoint.x,connectionPoint.y,connectionPoint.z) *PhysEnv::WORLD_SCALE,
		btVector3(wheelDir.x,wheelDir.y,wheelDir.z),
		btVector3(wheelAxle.x,wheelAxle.y,wheelAxle.z),
		suspensionRestLength,wheelRadius *PhysEnv::WORLD_SCALE,
		m_tuning,bIsFrontWheel
	);
	UNUSED(info);
}
void BaseVehicleComponent::InitializeWheels()
{
	for(auto &data : m_wheels)
		InitializeWheel(data);
}

Bool BaseVehicleComponent::AddWheel(const Vector3 &connectionPoint,const Vector3 &wheelAxle,Bool bIsFrontWheel,UChar *wheelId,const Vector3 &mdlOffset,const Quat &mdlRotOffset)
{
	m_wheels.push_back({});
	auto &data = m_wheels.back();
	*wheelId = GetWheelCount() -1;
	data.bIsFrontWheel = bIsFrontWheel;
	data.connectionPoint = connectionPoint;
	data.wheelAxle = wheelAxle;
	data.modelTranslation = mdlOffset;
	data.modelRotation = mdlRotOffset;
	InitializeWheel(data);
	return true;
}

void BaseVehicleComponent::InitializeWheelEntity(pragma::BaseWheelComponent *wheel,const WheelData &data)
{
	wheel->SetWheelDirection(GetWheelDirection());
	wheel->SetMaxSuspensionLength(GetMaxSuspensionLength());
	wheel->SetMaxSuspensionCompression(GetMaxSuspensionCompression());
	wheel->SetWheelRadius(GetWheelRadius());
	wheel->SetSuspensionStiffness(GetSuspensionStiffness());
	wheel->SetWheelDampingCompression(GetWheelDampingCompression());
	wheel->SetFrictionSlip(GetFrictionSlip());
	wheel->SetSteeringAngle(GetSteeringAngle());
	wheel->SetRollInfluence(GetRollInfluence());
	wheel->SetModelTranslation(data.modelTranslation);
	wheel->SetModelRotation(data.modelRotation);
}

BaseEntity *BaseVehicleComponent::GetSteeringWheel() {return m_steeringWheel.get();}
Float BaseVehicleComponent::GetMaxEngineForce() const {return m_maxEngineForce;}
Float BaseVehicleComponent::GetMaxReverseEngineForce() const {return m_maxReverseEngineForce;}
Float BaseVehicleComponent::GetMaxBrakeForce() const {return m_maxBrakeForce;}
void BaseVehicleComponent::SetMaxEngineForce(Float force) {m_maxEngineForce = force;}
void BaseVehicleComponent::SetMaxReverseEngineForce(Float force) {m_maxReverseEngineForce = force;}
void BaseVehicleComponent::SetMaxBrakeForce(Float force) {m_maxBrakeForce = force;}
void BaseVehicleComponent::SetAcceleration(Float acc) {m_acceleration = acc;}
void BaseVehicleComponent::SetTurnSpeed(Float speed) {m_turnSpeed = speed;}
void BaseVehicleComponent::SetMaxTurnAngle(Float ang) {m_maxTurnAngle = ang;}

void BaseVehicleComponent::Think(double tDelta)
{
	if(m_vhcRaycast == nullptr)
		return;
	m_vhcRaycast->updateVehicle(tDelta);
	if(HasDriver())
	{
		auto *driver = GetDriver();
		if(driver->IsPlayer())
		{
			auto plComponent = driver->GetPlayerComponent();
			auto bResetMovement = true;
			if(plComponent->GetActionInput(Action::MoveForward))
			{
				auto max = GetMaxEngineForce();
				auto acc = GetAcceleration();
				auto force = GetEngineForce();
				if(force < max)
				{
					force = umath::min(force +acc *CFloat(tDelta),max);
					SetEngineForce(force);
				}
				bResetMovement = false;
			}
			if(plComponent->GetActionInput(Action::Jump))
			{
				auto max = GetMaxBrakeForce();
				auto acc = 4000.f;//GetAcceleration();
				auto force = GetBrakeForce();
				if(force < max)
				{
					force = umath::min(force +acc *CFloat(tDelta),max);
					SetBrakeForce(force);
				}
			}
			else
			{
				SetBrakeForce(0.f);
				if(plComponent->GetActionInput(Action::MoveBackward))
				{
					auto min = -GetMaxReverseEngineForce();
					auto acc = GetAcceleration();
					auto force = GetEngineForce();
					if(force > min)
					{
						force = umath::max(force -acc *CFloat(tDelta),min);
						SetEngineForce(force);
					}
					bResetMovement = false;
				}
			}
			if(bResetMovement == true)
			{
				auto acc = GetAcceleration();
				auto force = GetEngineForce();
				force = umath::approach(force,0.f,acc *CFloat(tDelta));
				SetEngineForce(force);
			}
			auto bResetSteering = true;
			if(plComponent->GetActionInput(Action::MoveLeft))
			{
				auto max = GetMaxTurnAngle();
				auto turnSpeed = GetTurnSpeed();
				auto ang = GetSteeringAngle();
				if(ang < max)
				{
					ang = umath::min(ang +turnSpeed *CFloat(tDelta),max);
					SetSteeringAngle(ang);
				}
				bResetSteering = false;
			}
			if(plComponent->GetActionInput(Action::MoveRight))
			{
				auto min = -GetMaxTurnAngle();
				auto turnSpeed = GetTurnSpeed();
				auto ang = GetSteeringAngle();
				if(ang > min)
				{
					ang = umath::max(ang -turnSpeed *CFloat(tDelta),min);
					SetSteeringAngle(ang);
				}
				bResetSteering = false;
			}
			if(bResetSteering == true)
			{
				auto turnSpeed = GetTurnSpeed();
				auto ang = GetSteeringAngle();
				ang = umath::approach(ang,0.f,turnSpeed *CFloat(tDelta));
				SetSteeringAngle(ang);
			}
		}
	}
}

btWheelInfo *BaseVehicleComponent::GetWheelInfo(int wheel)
{
	if(m_vhcRaycast == nullptr)
		return nullptr;
	if(wheel >= m_vhcRaycast->getNumWheels())
		return nullptr;
	return &m_vhcRaycast->getWheelInfo(wheel);
}

void BaseVehicleComponent::SetEngineForce(Float force)
{
	m_engineForce = force;
	if(m_vhcRaycast == nullptr)
		return;
	for(auto i=0;i<m_vhcRaycast->getNumWheels();i++)
		m_vhcRaycast->applyEngineForce(force *PhysEnv::WORLD_SCALE,i);
}

void BaseVehicleComponent::SetBrakeForce(Float force)
{
	m_brakeForce = force;
	if(m_vhcRaycast == nullptr)
		return;
	for(auto i=0;i<m_vhcRaycast->getNumWheels();i++)
		m_vhcRaycast->setBrake(force *PhysEnv::WORLD_SCALE,i);
}

void BaseVehicleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto pPhysComponent = GetEntity().GetPhysicsComponent();
		if(pPhysComponent.expired())
			return;
		pPhysComponent->AddCollisionFilter(CollisionMask::Vehicle);
	});

	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter() | CollisionMask::Vehicle);
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent*>(whRenderComponent.get())->SetCastShadows(true);
}

unsigned char BaseVehicleComponent::GetWheelCount() {return static_cast<uint8_t>(m_wheels.size());}

Vector3 &BaseVehicleComponent::GetWheelDirection() {return m_wheelInfo.wheelDirection;}
void BaseVehicleComponent::SetWheelDirection(const Vector3 &dir)
{
	m_wheelInfo.wheelDirection = dir;
	for(auto it=m_wheels.begin();it!=m_wheels.end();++it)
	{
		auto &data = *it;
		if(data.hWheel.valid())
			data.hWheel->SetWheelDirection(dir);
	}
}
Float BaseVehicleComponent::GetMaxSuspensionLength() const {return m_wheelInfo.suspensionLength;}
void BaseVehicleComponent::SetMaxSuspensionLength(Float len)
{
	m_wheelInfo.suspensionLength = len;
	for(auto it=m_wheels.begin();it!=m_wheels.end();++it)
	{
		auto &data = *it;
		if(data.hWheel.valid())
			data.hWheel->SetMaxSuspensionLength(len);
	}
}
Float BaseVehicleComponent::GetMaxSuspensionCompression() const {return m_wheelInfo.suspensionCompression;}
void BaseVehicleComponent::SetMaxSuspensionCompression(Float cmp)
{
	m_wheelInfo.suspensionCompression = cmp;
	for(auto it=m_wheels.begin();it!=m_wheels.end();++it)
	{
		auto &data = *it;
		if(data.hWheel.valid())
			data.hWheel->SetMaxSuspensionCompression(cmp);
	}
}
Float BaseVehicleComponent::GetMaxDampingRelaxation() const {return m_wheelInfo.dampingRelaxation;}
void BaseVehicleComponent::SetMaxDampingRelaxation(Float damping)
{
	m_wheelInfo.dampingRelaxation = damping;
	for(auto it=m_wheels.begin();it!=m_wheels.end();++it)
	{
		auto &data = *it;
		if(data.hWheel.valid())
			data.hWheel->SetMaxDampingRelaxation(damping);
	}
}
Float BaseVehicleComponent::GetWheelRadius() const {return m_wheelInfo.wheelRadius;}
void BaseVehicleComponent::SetWheelRadius(Float radius)
{
	m_wheelInfo.wheelRadius = radius;
	for(auto it=m_wheels.begin();it!=m_wheels.end();++it)
	{
		auto &data = *it;
		if(data.hWheel.valid())
			data.hWheel->SetWheelRadius(radius);
	}
}
Float BaseVehicleComponent::GetSuspensionStiffness() const {return m_wheelInfo.suspensionStiffness;}
void BaseVehicleComponent::SetSuspensionStiffness(Float stiffness)
{
	m_wheelInfo.suspensionStiffness = stiffness;
	for(auto it=m_wheels.begin();it!=m_wheels.end();++it)
	{
		auto &data = *it;
		if(data.hWheel.valid())
			data.hWheel->SetSuspensionStiffness(stiffness);
	}
}
Float BaseVehicleComponent::GetWheelDampingCompression() const {return m_wheelInfo.wheelDampingCompression;}
void BaseVehicleComponent::SetWheelDampingCompression(Float cmp)
{
	m_wheelInfo.wheelDampingCompression = cmp;
	for(auto it=m_wheels.begin();it!=m_wheels.end();++it)
	{
		auto &data = *it;
		if(data.hWheel.valid())
			data.hWheel->SetWheelDampingCompression(cmp);
	}
}
Float BaseVehicleComponent::GetFrictionSlip() const {return m_wheelInfo.frictionSlip;}
void BaseVehicleComponent::SetFrictionSlip(Float slip)
{
	m_wheelInfo.frictionSlip = slip;
	for(auto it=m_wheels.begin();it!=m_wheels.end();++it)
	{
		auto &data = *it;
		if(data.hWheel.valid())
			data.hWheel->SetFrictionSlip(slip);
	}
}
Float BaseVehicleComponent::GetSteeringAngle() const {return m_wheelInfo.steeringAngle;}
void BaseVehicleComponent::SetSteeringAngle(Float ang)
{
	m_wheelInfo.steeringAngle = ang;
	for(auto it=m_wheels.begin();it!=m_wheels.end();++it)
	{
		auto &data = *it;
		if(data.hWheel.valid())
		{
			auto &wheel = data.hWheel;
			if(wheel->IsFrontWheel())
				wheel->SetSteeringAngle(ang);
		}
	}
}
Float BaseVehicleComponent::GetRollInfluence() const {return m_wheelInfo.rollInfluence;}
void BaseVehicleComponent::SetRollInfluence(Float influence)
{
	m_wheelInfo.rollInfluence = influence;
	for(auto it=m_wheels.begin();it!=m_wheels.end();++it)
	{
		auto &data = *it;
		if(data.hWheel.valid())
			data.hWheel->SetRollInfluence(influence);
	}
}
