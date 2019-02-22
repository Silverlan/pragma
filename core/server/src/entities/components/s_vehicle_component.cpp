#include "stdafx_server.h"
#include "pragma/entities/components/s_vehicle_component.hpp"
#include "pragma/entities/components/s_wheel_component.hpp"
#include "pragma/entities/prop/s_prop_physics.h"
#include "pragma/entities/prop/s_prop_dynamic.hpp"
#include "pragma/entities/player.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/entities/components/s_wheel_component.hpp"
#include "pragma/entities/components/s_attachable_component.hpp"
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/physics/physenvironment.h>

using namespace pragma;

extern DLLSERVER SGame *s_game;

std::vector<SVehicleComponent*> SVehicleComponent::s_vehicles;
const std::vector<SVehicleComponent*> &SVehicleComponent::GetAll() {return s_vehicles;}
unsigned int SVehicleComponent::GetVehicleCount() {return CUInt32(s_vehicles.size());}

SVehicleComponent::SVehicleComponent(BaseEntity &ent)
	: BaseVehicleComponent(ent),SBaseNetComponent()
{
	static_cast<SBaseEntity&>(ent).SetShared(true);
	s_vehicles.push_back(this);
}

SVehicleComponent::~SVehicleComponent()
{
	auto it = std::find(s_vehicles.begin(),s_vehicles.end(),this);
	if(it != s_vehicles.end())
		s_vehicles.erase(it);
}
luabind::object SVehicleComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SVehicleComponentHandleWrapper>(l);}
void SVehicleComponent::OnRemove() {ClearDriver();}
void SVehicleComponent::OnUse(BaseEntity *pl)
{
	if(HasDriver())
		return;
	SetDriver(pl);
}

void SVehicleComponent::ClearDriver()
{
	auto *driver = BaseVehicleComponent::GetDriver();
	if(driver->IsCharacter())
		driver->GetCharacterComponent()->SetFrozen(false);
	BaseVehicleComponent::ClearDriver();
	if(m_playerAction.IsValid())
		m_playerAction.Remove();
	NetPacket p;
	nwm::write_entity(p,nullptr);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEventTCP(NET_EVENT_VEHICLE_SET_DRIVER,p);
}
void SVehicleComponent::OnActionInput(Action action, bool b)
{
	if(action == Action::Use)
	{
		if(b == false)
			return;
		ClearDriver();
	}
}
void SVehicleComponent::Initialize()
{
	BaseVehicleComponent::Initialize();

	BindEvent(UsableComponent::EVENT_CAN_USE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &bCanUse = static_cast<CECanUseData&>(evData.get()).canUse;
		bCanUse = !HasDriver();
		return util::EventReply::Handled;
	});
	BindEventUnhandled(UsableComponent::EVENT_ON_USE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnUse(static_cast<CEOnUseData&>(evData.get()).entity);
	});
	BindEventUnhandled(BaseEntity::EVENT_ON_POST_SPAWN,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnPostSpawn();
	});

	auto &ent = GetEntity();
	ent.AddComponent<UsableComponent>();
}
void SVehicleComponent::SetDriver(BaseEntity *ent)
{
	if(ent == GetDriver())
		return;
	BaseVehicleComponent::SetDriver(ent);
	if(ent->IsCharacter())
		ent->GetCharacterComponent()->SetFrozen(true);
	if(ent->IsPlayer())
	{
		auto plComponent = ent->GetPlayerComponent();
		m_playerAction = plComponent->BindEvent(SPlayerComponent::EVENT_HANDLE_ACTION_INPUT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			auto &actionData = static_cast<CEHandleActionInput&>(evData.get());
			if(actionData.action == Action::Use)
			{
				OnActionInput(actionData.action,actionData.pressed);
				return util::EventReply::Handled;
			}
			return util::EventReply::Unhandled;
		});
	}
	auto &entThis = static_cast<SBaseEntity&>(GetEntity());
	if(!entThis.IsShared() || !entThis.IsSpawned())
		return;
	NetPacket p;
	nwm::write_entity(p,ent);
	entThis.SendNetEventTCP(NET_EVENT_VEHICLE_SET_DRIVER,p);
}

void SVehicleComponent::WriteWheelInfo(NetPacket &p,WheelData &data,btWheelInfo *info)
{
	auto conPoint = info->m_chassisConnectionPointCS /PhysEnv::WORLD_SCALE;
	auto &wheelDir = info->m_wheelDirectionCS;
	auto &axle = info->m_wheelAxleCS;
	auto &suspensionRest = info->m_suspensionRestLength1;
	auto &radius = info->m_wheelsRadius;
	auto &bFrontWheel = info->m_bIsFrontWheel;
	p->Write<Vector3>(Vector3(conPoint.x(),conPoint.y(),conPoint.z()));
	p->Write<Vector3>(Vector3(wheelDir.x(),wheelDir.y(),wheelDir.z()));
	p->Write<Vector3>(Vector3(axle.x(),axle.y(),axle.z()));
	p->Write<Float>(CFloat(suspensionRest));
	p->Write<Float>(CFloat(radius /PhysEnv::WORLD_SCALE));
	p->Write<Bool>(bFrontWheel);
	p->Write<Vector3>(data.modelTranslation);
	p->Write<Quat>(data.modelRotation);
}

void SVehicleComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<Float>(GetMaxEngineForce());
	packet->Write<Float>(GetMaxReverseEngineForce());
	packet->Write<Float>(GetMaxBrakeForce());
	packet->Write<Float>(GetAcceleration());
	packet->Write<Float>(GetTurnSpeed());
	packet->Write<Float>(GetMaxTurnAngle());
	packet->Write<bool>(IsFirstPersonCameraEnabled());
	packet->Write<bool>(IsThirdPersonCameraEnabled());
	packet->Write<Float>(GetBrakeForce());
	packet->Write<Float>(GetEngineForce());
	packet->Write<Float>(GetSteeringAngle());
	packet->Write<Float>(GetFrictionSlip());
	packet->Write<Float>(GetMaxSuspensionCompression());
	packet->Write<Float>(GetMaxSuspensionLength());
	packet->Write<Float>(GetRollInfluence());
	packet->Write<Float>(GetSuspensionStiffness());
	packet->Write<Float>(GetWheelDampingCompression());
	packet->Write<Vector3>(GetWheelDirection());
	packet->Write<Float>(GetWheelRadius());
	packet->Write<WheelInfo>(m_wheelInfo);
	nwm::write_entity(packet,GetSteeringWheel());
	auto numWheels = GetWheelCount();
	packet->Write<UChar>(numWheels);
	for(UChar i=0;i<numWheels;i++)
	{
		auto *info = GetWheelInfo(i);
		WriteWheelInfo(packet,m_wheels[i],info);
	}
}

BaseEntity *SVehicleComponent::AddWheel(const std::string &mdl,const Vector3 &connectionPoint,const Vector3 &wheelAxle,Bool bIsFrontWheel,const Vector3 &mdlOffset,const Quat &mdlRotOffset)
{
	UChar wheelId = 0;
	if(AddWheel(connectionPoint,wheelAxle,bIsFrontWheel,&wheelId,mdlOffset,mdlRotOffset) == false)
		return nullptr;
	auto *ent = s_game->CreateEntity<SWheel>();
	if(ent == nullptr)
		return nullptr;
	ent->SetSynchronized(false);
	auto pWheelComponent = ent->GetComponent<pragma::SWheelComponent>();
	if(pWheelComponent.valid())
	{
		pWheelComponent->SetChassisConnectionPoint(connectionPoint);
		pWheelComponent->SetWheelAxle(wheelAxle);
		pWheelComponent->SetFrontWheel(bIsFrontWheel);
		pWheelComponent->Attach(&GetEntity(),wheelId);
	}
	if(!mdl.empty())
	{
		auto pMdlComponent = ent->GetModelComponent();
		if(pMdlComponent.valid())
			pMdlComponent->SetModel(mdl.c_str());
	}
	ent->Spawn();
	GetEntity().RemoveEntityOnRemoval(ent);
	return ent;
}

void SVehicleComponent::SetSteeringWheelModel(const std::string &mdl)
{
	m_steeringWheelMdl = mdl;
	auto &entThis = static_cast<SBaseEntity&>(GetEntity());
	if(!entThis.IsSpawned())
		return;
	if(!m_steeringWheel.IsValid())
	{
		auto *ent = s_game->CreateEntity<PropDynamic>();
		auto mdlComponent = ent->GetModelComponent();
		if(mdlComponent.valid())
			mdlComponent->SetModel(mdl.c_str());
		auto pAttachableComponent = ent->AddComponent<SAttachableComponent>();
		if(pAttachableComponent.valid())
		{
			AttachmentInfo attInfo {};
			attInfo.flags |= FAttachmentMode::SnapToOrigin | FAttachmentMode::UpdateEachFrame;
			pAttachableComponent->AttachToAttachment(&GetEntity(),"steering_wheel",attInfo);
		}
		ent->SetSynchronized(false);
		ent->Spawn();
		m_steeringWheel = ent->GetHandle();
		entThis.RemoveEntityOnRemoval(ent);
		InitializeSteeringWheel();
		NetPacket p;
		nwm::write_entity(p,ent);
		entThis.SendNetEventTCP(NET_EVENT_VEHICLE_SET_STEERING_WHEEL,p);
		return;
	}
	BaseVehicleComponent::SetSteeringWheelModel(mdl);
}

BaseEntity *SVehicleComponent::AddWheel(const Vector3 &connectionPoint,const Vector3 &wheelAxle,Bool bIsFrontWheel,const Vector3 &mdlOffset,const Quat &mdlRotOffset)
{
	return AddWheel("",connectionPoint,wheelAxle,bIsFrontWheel,mdlOffset,mdlRotOffset);
}

Bool SVehicleComponent::AddWheel(const Vector3 &connectionPoint,const Vector3 &wheelAxle,Bool bIsFrontWheel,UChar *wheelId,const Vector3 &mdlOffset,const Quat &mdlRotOffset)
{
	if(BaseVehicleComponent::AddWheel(connectionPoint,wheelAxle,bIsFrontWheel,wheelId,mdlOffset,mdlRotOffset) == false)
		return false;
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() && ent.IsSpawned())
	{
		auto *info = GetWheelInfo(GetWheelCount() -1);
		if(info != nullptr)
		{
			NetPacket p;
			WriteWheelInfo(p,m_wheels.back(),info);
			ent.SendNetEventTCP(NET_EVENT_VEHICLE_ADD_WHEEL,p);
		}
	}
	return true;
}

void SVehicleComponent::SetMaxEngineForce(Float force)
{
	BaseVehicleComponent::SetMaxEngineForce(force);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsNetworked())
		return;
	NetPacket p;
	p->Write<Float>(force);
	ent.SendNetEventTCP(NET_EVENT_VEHICLE_MAX_ENGINE_FORCE,p);
}
void SVehicleComponent::SetMaxReverseEngineForce(Float force)
{
	BaseVehicleComponent::SetMaxReverseEngineForce(force);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsNetworked())
		return;
	NetPacket p;
	p->Write<Float>(force);
	ent.SendNetEventTCP(NET_EVENT_VEHICLE_MAX_REVERSE_ENGINE_FORCE,p);
}
void SVehicleComponent::SetMaxBrakeForce(Float force)
{
	BaseVehicleComponent::SetMaxBrakeForce(force);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsNetworked())
		return;
	NetPacket p;
	p->Write<Float>(force);
	ent.SendNetEventTCP(NET_EVENT_VEHICLE_MAX_BRAKE_FORCE,p);
}
void SVehicleComponent::SetAcceleration(Float acc)
{
	BaseVehicleComponent::SetAcceleration(acc);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsNetworked())
		return;
	NetPacket p;
	p->Write<Float>(acc);
	ent.SendNetEventTCP(NET_EVENT_VEHICLE_ACCELERATION,p);
}
void SVehicleComponent::SetTurnSpeed(Float speed)
{
	BaseVehicleComponent::SetTurnSpeed(speed);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsNetworked())
		return;
	NetPacket p;
	p->Write<Float>(speed);
	ent.SendNetEventTCP(NET_EVENT_VEHICLE_TURN_SPEED,p);
}
void SVehicleComponent::SetMaxTurnAngle(Float ang)
{
	BaseVehicleComponent::SetMaxTurnAngle(ang);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsNetworked())
		return;
	NetPacket p;
	p->Write<Float>(ang);
	ent.SendNetEventTCP(NET_EVENT_VEHICLE_MAX_TURN_ANGLE,p);
}

void SVehicleComponent::SetFirstPersonCameraEnabled(bool b)
{
	BaseVehicleComponent::SetFirstPersonCameraEnabled(b);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsNetworked())
		return;
	NetPacket p;
	p->Write<bool>(b);
	ent.SendNetEventTCP(NET_EVENT_VEHICLE_SET_FIRST_PERSON_CAMERA_ENABLED,p);
}
void SVehicleComponent::SetThirdPersonCameraEnabled(bool b)
{
	BaseVehicleComponent::SetThirdPersonCameraEnabled(b);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsNetworked())
		return;
	NetPacket p;
	p->Write<bool>(b);
	ent.SendNetEventTCP(NET_EVENT_VEHICLE_SET_THIRD_PERSON_CAMERA_ENABLED,p);
}

void SVehicleComponent::OnPostSpawn()
{
	if(!m_steeringWheelMdl.empty())
	{
		SetSteeringWheelModel(m_steeringWheelMdl);
		m_steeringWheelMdl.clear();
	}
}
