#include "stdafx_shared.h"
#include "pragma/entities/components/base_vehicle_component.hpp"
#include "pragma/entities/components/base_wheel_component.hpp"
#include "pragma/basevehicle_raycaster.h"
#include "pragma/physics/environment.hpp"
#include "pragma/input/inkeys.h"
#include "pragma/model/model.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_attachable_component.hpp"
#include "pragma/physics/raytraces.h"

//#define ENABLE_DEPRECATED_PHYSICS

using namespace pragma;

#pragma optimize("",off)
BaseVehicleComponent::BaseVehicleComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}

void BaseVehicleComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_physVehicle.IsValid())
		m_physVehicle.Remove();
	for(auto &wheel : m_wheels)
	{
		if(wheel.hWheel.expired())
			continue;
		wheel.hWheel->GetEntity().Remove();
	}
}

#include "pragma/physics/shape.hpp"
enum
{
	eFRONT_LEFT = 0,
	eFRONT_RIGHT = 1,
	eREAR_LEFT = 2,
	eREAR_RIGHT = 3
};
static std::shared_ptr<pragma::physics::IConvexHullShape> create_chassis_mesh(pragma::physics::IEnvironment &env,const Vector3 &dims,float mass,pragma::physics::IMaterial &mat)
{
	auto x = dims.x *0.5;
	auto y = dims.y *0.5;
	auto z = dims.z *0.5;

	auto shape = env.CreateConvexHullShape(mat);
	shape->ReservePoints(8);
	shape->AddPoint(Vector3(x,y,-z));
	shape->AddPoint(Vector3(x,y,z));
	shape->AddPoint(Vector3(x,-y,z));
	shape->AddPoint(Vector3(x,-y,-z));
	shape->AddPoint(Vector3(-x,y,-z));
	shape->AddPoint(Vector3(-x,y,z));
	shape->AddPoint(Vector3(-x,-y,z));
	shape->AddPoint(Vector3(-x,-y,-z));
	shape->Build();
	shape->SetMass(mass);
	return shape;
}

static std::shared_ptr<pragma::physics::IConvexHullShape> create_wheel_mesh(pragma::physics::IEnvironment &env,float width,float radius,float mass,pragma::physics::IMaterial &mat)
{
	auto shape = env.CreateConvexHullShape(mat);
	shape->ReservePoints(32);
	for(auto i=0;i<16;++i)
	{
		auto cosTheta = umath::cos(i *umath::pi *2.0 /16.0);
		auto sinTheta = umath::sin(i *umath::pi *2.0 /16.0);
		auto y = radius*cosTheta;
		auto z = radius*sinTheta;
		shape->AddPoint(Vector3(-width/2.0, y, z));
		shape->AddPoint(Vector3(width/2.0, y, z));
	}
	shape->Build();
	shape->SetMass(mass);
	return shape;
}

static std::array<Vector3,4> computeWheelCenterActorOffsets4W(float wheelFrontZ,float wheelRearZ,const Vector3 &chassisDims,float wheelWidth,float wheelRadius,uint32_t numWheels)
{
	std::array<Vector3,4> wheelCentreOffsets = {};
	//chassisDims.z is the distance from the rear of the chassis to the front of the chassis.
	//The front has z = 0.5*chassisDims.z and the rear has z = -0.5*chassisDims.z.
	//Compute a position for the front wheel and the rear wheel along the z-axis.
	//Compute the separation between each wheel along the z-axis.
	auto numLeftWheels = numWheels/2.0;
	auto deltaZ = (wheelFrontZ - wheelRearZ)/(numLeftWheels-1.0);
	//Set the outside of the left and right wheels to be flush with the chassis.
	//Set the top of the wheel to be just touching the underside of the chassis.
	//Begin by setting the rear-left/rear-right/front-left,front-right wheels.
	wheelCentreOffsets[eREAR_LEFT] = Vector3((-chassisDims.x + wheelWidth)*0.5, -(chassisDims.y/2 + wheelRadius), wheelRearZ + 0*deltaZ*0.5);
	wheelCentreOffsets[eREAR_RIGHT] = Vector3((chassisDims.x - wheelWidth)*0.5, -(chassisDims.y/2 + wheelRadius), wheelRearZ + 0*deltaZ*0.5);
	wheelCentreOffsets[eFRONT_LEFT] = Vector3((-chassisDims.x + wheelWidth)*0.5, -(chassisDims.y/2 + wheelRadius), wheelRearZ + (numLeftWheels-1)*deltaZ);
	wheelCentreOffsets[eFRONT_RIGHT] = Vector3((chassisDims.x - wheelWidth)*0.5, -(chassisDims.y/2 + wheelRadius), wheelRearZ + (numLeftWheels-1)*deltaZ);
	//Set the remaining wheels.
	auto wheelCount = 4;
	for(auto i=2;i<(numWheels -2);i+=2)
	{
		wheelCentreOffsets[wheelCount + 1] = Vector3((-chassisDims.x + wheelWidth)*0.5, -(chassisDims.y/2 + wheelRadius), wheelRearZ + i*deltaZ*0.5);
		wheelCentreOffsets[wheelCount + 2] = Vector3((chassisDims.x - wheelWidth)*0.5, -(chassisDims.y/2 + wheelRadius), wheelRearZ + i*deltaZ*0.5);

		wheelCount = wheelCount +2;
	}
	return wheelCentreOffsets;
}

void BaseVehicleComponent::InitializeVehiclePhysics(PHYSICSTYPE type,BasePhysicsComponent::PhysFlags flags)
{
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return;
	auto mdlComponent = ent.GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(mdl == nullptr || pPhysComponent.expired())
		return;
	auto &colMeshes = mdl->GetCollisionMeshes();
	if(colMeshes.empty())
		return;
	//auto &chassisColMesh = colMeshes.front();
	// TODO: Proper mess
	float chassisMass = mdl->GetMass();
	//chassisColMesh->SetMass(chassisMass);
	//chassisColMesh->UpdateShape();

	auto gLengthScale = 40.0;

#ifdef VHC_USE_MESH_CHASSIS
	auto wheelWidth = 7.51668;
	auto wheelRadius = 19.3685;
#else
	auto wheelWidth = 16.0;
	auto wheelRadius = 20.0;
#endif
	auto wheelMass = 20.0;
	auto mat = physEnv->CreateMaterial(0.5f,0.5f,0.6f);

	std::vector<pragma::physics::IShape*> chassisShapes {};
	chassisShapes.reserve(colMeshes.size());
	for(auto &colMesh : colMeshes)
	{
		chassisShapes.push_back(colMesh->GetShape().get());
		//break;
	}


	//auto chassisMesh = chassisColMesh->GetShape();
	//
#ifdef VHC_USE_MESH_CHASSIS
	auto chassisMesh = physEnv->CreateCompoundShape(chassisShapes);
#else
	auto chassisDims = Vector3(2.5,2.0,5.0);
	chassisMass = 1500.0;
	auto moi = Vector3(
	(chassisDims.y*chassisDims.y + chassisDims.z*chassisDims.z)*chassisMass/12.0,
		(chassisDims.x*chassisDims.x + chassisDims.z*chassisDims.z)*0.8*chassisMass/12.0,
		(chassisDims.x*chassisDims.x + chassisDims.y*chassisDims.y)*chassisMass/12.0
	);
	auto cmOffset = Vector3(0.0, -chassisDims.y*0.5 + 0.65, 0.25) *static_cast<float>(gLengthScale);
	chassisDims = chassisDims *static_cast<float>(gLengthScale);
auto chassisMesh = create_chassis_mesh(*physEnv,chassisDims,chassisMass,*mat);
#endif

	auto mdlWheel = GetEntity().GetNetworkState()->GetGameState()->LoadModel("vehicles/viperscout_wheel.wmd");
	Con::cout<<"Wheel mass: "<<mdlWheel->GetMass()<<Con::endl;
	// TODO: Make sure correct mass, width, radius!
	auto wheelMesh = mdlWheel->GetCollisionMeshes().front()->GetShape();

	//Vector3 wheelMin,wheelMax;
	//wheelMesh->GetAABB(wheelMin,wheelMax);
	//Con::cout<<"Bounds: ("<<wheelMin.x<<","<<wheelMin.y<<","<<wheelMin.z<<") ("<<wheelMax.x<<","<<wheelMax.y<<","<<wheelMax.z<<")"<<Con::endl;
	// wheelMesh = create_wheel_mesh(*physEnv,wheelWidth,wheelRadius,wheelMass,*mat);

	auto numWheels = 4;

	//Set up the chassis mass, dimensions, moment of inertia, and center of mass offset.
	//The moment of inertia is just the moment of inertia of a cuboid but modified for easier steering.
	//Center of mass offset is 0.65m above the base of the chassis and 0.25m towards the front.


#ifdef VHC_USE_MESH_CHASSIS
	std::array<Vector3,4> wheelCenterActorOffsets {};
	auto attTransform = mdl->CalcReferenceAttachmentPose(mdl->LookupAttachment("wheel_fl"));
	if(attTransform.has_value())
		wheelCenterActorOffsets.at(umath::to_integral(physics::VehicleCreateInfo::Wheel::FrontLeft)) = attTransform->GetOrigin();

	attTransform = mdl->CalcReferenceAttachmentPose(mdl->LookupAttachment("wheel_fr"));
	if(attTransform.has_value())
		wheelCenterActorOffsets.at(umath::to_integral(physics::VehicleCreateInfo::Wheel::FrontRight)) = attTransform->GetOrigin();

	attTransform = mdl->CalcReferenceAttachmentPose(mdl->LookupAttachment("wheel_bl"));
	if(attTransform.has_value())
		wheelCenterActorOffsets.at(umath::to_integral(physics::VehicleCreateInfo::Wheel::RearLeft)) = attTransform->GetOrigin();

	attTransform = mdl->CalcReferenceAttachmentPose(mdl->LookupAttachment("wheel_br"));
	if(attTransform.has_value())
		wheelCenterActorOffsets.at(umath::to_integral(physics::VehicleCreateInfo::Wheel::RearRight)) = attTransform->GetOrigin();
#else
	auto frontZ = chassisDims.z*0.3;
	auto rearZ = -chassisDims.z*0.3;
	auto wheelCenterActorOffsets = computeWheelCenterActorOffsets4W(frontZ, rearZ, chassisDims, wheelWidth, wheelRadius, numWheels);
#endif

#ifdef VHC_USE_MESH_CHASSIS
#ifndef VHC_RELEASE_MODE
	for(auto &offset : wheelCenterActorOffsets)
	{
		// TODO: Why?
		offset.x = -offset.x;
		//offset.z = -offset.z;
	}
#endif
#endif

	//for idx,att in ipairs({"wheel_fr","wheel_fl","wheel_br","wheel_bl"}) do

	auto vhcCreateInfo = physics::VehicleCreateInfo::CreateStandardFourWheelDrive(
		wheelCenterActorOffsets,
		wheelWidth, // Wheel width
		wheelRadius // Wheel radius
	);

	m_wheels.push_back({});
	m_wheels.back().model = "vehicles/viperscout_wheel.wmd";
	m_wheels.push_back(m_wheels.back());
	m_wheels.push_back(m_wheels.back());
	m_wheels.push_back(m_wheels.back());

	physics::PhysObjCreateInfo physObjCreateInfo {};
	auto numChassisMeshes = physObjCreateInfo.AddShape(*chassisMesh);
	auto wheelMeshOffset = numChassisMeshes;
	physObjCreateInfo.AddShape(*wheelMesh);
	physObjCreateInfo.AddShape(*wheelMesh);
	physObjCreateInfo.AddShape(*wheelMesh);
	physObjCreateInfo.AddShape(*wheelMesh);

	vhcCreateInfo.chassis.shapeIndices.reserve(numChassisMeshes);
	for(uint32_t i=0u;i<numChassisMeshes;++i)
		vhcCreateInfo.chassis.shapeIndices.push_back(i);
	for(auto i=0;i<vhcCreateInfo.wheels.size();++i)
		vhcCreateInfo.wheels[i].shapeIndex = i +wheelMeshOffset;

	auto plane = physEnv->CreatePlane(Vector3(0,1,0),0.f,*mat);
	plane->Spawn();
	plane->SetCollisionFilterGroup(CollisionMask::Default);
	plane->SetCollisionFilterMask(CollisionMask::All);

	//std::vector<physics::IShape*> shapes = {chassisMesh.get(),wheelMesh.get(),wheelMesh.get(),wheelMesh.get(),wheelMesh.get()};
	//auto shape = physEnv->CreateCompoundShape(shapes);

	// TODO: Create physics object

	auto physObj = pPhysComponent->InitializePhysics(physObjCreateInfo,flags);
	if(physObj.expired())
		return;
	auto *colObj = physObj->GetCollisionObject();
	if(colObj == nullptr || colObj->IsRigid() == false)
		return;
	auto *pRigidBody = colObj->GetRigidBody();
	//util::WeakHandle<PhysObj> InitializePhysics(const physics::PhysObjCreateInfo &physObjCreateInfo,PhysFlags flags,int32_t rootMeshBoneId=-1);

	//auto rigidBody = physEnv->CreateRigidBody(*shape,true);
	vhcCreateInfo.actor = util::shared_handle_cast<physics::IBase,physics::IRigidBody>(pRigidBody->ClaimOwnership());
	pRigidBody->SetCollisionFilterGroup(CollisionMask::Vehicle);
	pRigidBody->SetCollisionFilterMask(CollisionMask::All);
#ifndef VHC_USE_MESH_CHASSIS
	pRigidBody->SetCenterOfMassOffset(cmOffset);
#endif

	auto vhc = physEnv->CreateVehicle(vhcCreateInfo);
	vhc->SetGear(physics::IVehicle::Gear::First);
	vhc->SetUseAutoGears(true);
	vhc->SetBrakeFactor(1.0);
	vhc->SetUseDigitalInputs(true);
	vhc->Spawn();
	m_physVehicle = vhc;

	m_vhcCreateInfo = vhcCreateInfo;
	InitializeWheelEntities();
}

void BaseVehicleComponent::DestroyVehiclePhysics()
{
	if(m_physVehicle.IsValid())
		m_physVehicle.Remove();
}

bool BaseVehicleComponent::IsFirstPersonCameraEnabled() const {return m_bFirstPersonCameraEnabled;}
bool BaseVehicleComponent::IsThirdPersonCameraEnabled() const {return m_bThirdPersonCameraEnabled;}
void BaseVehicleComponent::SetFirstPersonCameraEnabled(bool b) {m_bFirstPersonCameraEnabled = b;}
void BaseVehicleComponent::SetThirdPersonCameraEnabled(bool b) {m_bThirdPersonCameraEnabled = b;}

physics::IVehicle *BaseVehicleComponent::GetPhysicsVehicle() {return m_physVehicle.Get();}
const physics::IVehicle *BaseVehicleComponent::GetPhysicsVehicle() const {return const_cast<BaseVehicleComponent*>(this)->GetPhysicsVehicle();}

float BaseVehicleComponent::GetSpeedKmh() const
{
	// TODO
	return 0.f;
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

Bool BaseVehicleComponent::AddWheel(const Vector3 &connectionPoint,const Vector3 &wheelAxle,Bool bIsFrontWheel,UChar *wheelId,const Vector3 &mdlOffset,const Quat &mdlRotOffset)
{
#if 0
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
#endif
	return false;
}

BaseWheelComponent *BaseVehicleComponent::CreateWheelEntity(uint8_t wheelIndex)
{
	auto *entWheel = GetEntity().GetNetworkState()->GetGameState()->CreateEntity("vhc_wheel");
	if(entWheel == nullptr)
		return nullptr;
	auto wheelComponent = entWheel->FindComponent("wheel");
	if(wheelComponent.expired())
		return nullptr;
	auto &hWheel = m_wheels.at(wheelIndex).hWheel = wheelComponent->GetHandle<BaseWheelComponent>();
	return hWheel.get();
}

void BaseVehicleComponent::InitializeWheelEntities()
{
	auto numWheels = umath::min(m_wheels.size(),m_vhcCreateInfo.wheels.size());
	for(auto i=decltype(numWheels){0u};i<numWheels;++i)
	{
		auto &wheelData = m_wheels.at(i);
		auto &wheelDesc = m_vhcCreateInfo.wheels.at(i);
		if(wheelData.model.empty())
			continue; // Wheel has no model, no need to create an entity for it
		auto *wheel = CreateWheelEntity(i);
		if(wheel == nullptr)
			continue;
		auto mdlComponent = wheel->GetEntity().GetModelComponent();
		if(mdlComponent.valid())
			mdlComponent->SetModel(wheelData.model);
		wheel->SetupWheel(*this,wheelDesc,i);
		wheel->GetEntity().Spawn();
		wheelData.hWheel = wheel->GetHandle<BaseWheelComponent>();
	}
}

BaseEntity *BaseVehicleComponent::GetSteeringWheel() {return m_steeringWheel.get();}

void BaseVehicleComponent::SetupVehicle(const pragma::physics::VehicleCreateInfo &createInfo,const std::string &wheelModel)
{
	//physics::VehicleCreateInfo::CreateStandardFourWheelDrive(wheelCenterOffsets,wheelWidth,wheelRadius);
}

void BaseVehicleComponent::Think(double tDelta)
{
#if 0
	if(m_vhcRaycast == nullptr)
		return;
	m_vhcRaycast->updateVehicle(tDelta);
	auto *driver = GetDriver();
	if(driver == nullptr || driver->IsPlayer() == false)
		return;
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
#endif
}
void BaseVehicleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BasePhysicsComponent::EVENT_INITIALIZE_PHYSICS,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto pPhysComponent = GetEntity().GetPhysicsComponent();
		if(pPhysComponent.expired())
			return util::EventReply::Unhandled;
		auto &physInitData = static_cast<CEInitializePhysics&>(evData.get());
		if(physInitData.physicsType != PHYSICSTYPE::DYNAMIC)
			return util::EventReply::Unhandled;
		InitializeVehiclePhysics(physInitData.physicsType,physInitData.flags);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		DestroyVehiclePhysics();
	});

	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter() | CollisionMask::Vehicle);
	ent.AddComponent("model");
	ent.AddComponent("physics");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent*>(whRenderComponent.get())->SetCastShadows(true);
}

uint8_t BaseVehicleComponent::GetWheelCount() const {return static_cast<uint8_t>(m_wheels.size());}

Float BaseVehicleComponent::GetSteeringAngle() const
{
	// TODO
	return 0.f;
	//return m_wheelInfo.steeringAngle;
}
#if 0
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
#endif
#pragma optimize("",on)
