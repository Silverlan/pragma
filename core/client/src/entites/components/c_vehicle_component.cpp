#include "stdafx_client.h"
#include "pragma/entities/components/c_vehicle_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/c_viewmodel.h"
#include "pragma/entities/c_viewbody.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_observable_component.hpp"
#include <pragma/input/inkeys.h>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

std::vector<CVehicleComponent*> CVehicleComponent::s_vehicles;
const std::vector<CVehicleComponent*> &CVehicleComponent::GetAll() {return s_vehicles;}
unsigned int CVehicleComponent::GetVehicleCount() {return CUInt32(s_vehicles.size());}

CVehicleComponent::CVehicleComponent(BaseEntity &ent)
	: BaseVehicleComponent(ent),m_bFirstPersonCamera(false)
{
	s_vehicles.push_back(this);
}

CVehicleComponent::~CVehicleComponent()
{
	if(m_hCbSteeringWheel.IsValid())
		m_hCbSteeringWheel.Remove();
	ClearDriver();
	for(int i=0;i<s_vehicles.size();i++)
	{
		if(s_vehicles[i] == this)
		{
			s_vehicles.erase(s_vehicles.begin() +i);
			break;
		}
	}
}

luabind::object CVehicleComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CVehicleHandle>(l);}

void CVehicleComponent::ReadWheelInfo(NetPacket &packet)
{
	auto conPoint = packet->Read<Vector3>();
	auto wheelDir = packet->Read<Vector3>();
	auto axle = packet->Read<Vector3>();
	auto suspensionRest = packet->Read<Float>();
	auto radius = packet->Read<Float>();
	auto bFrontWheel = packet->Read<Bool>();
	auto translation = packet->Read<Vector3>();
	auto rotation = packet->Read<Quat>();
	UChar wheelId = 0;
	if(AddWheel(conPoint,axle,bFrontWheel,&wheelId,translation,rotation) == false)
		return;
	auto *info = GetWheelInfo(wheelId);
	if(info == nullptr)
		return;
	info->m_wheelDirectionCS = btVector3(wheelDir.x,wheelDir.y,wheelDir.z);
	info->m_suspensionRestLength1 = suspensionRest;
	info->m_wheelsRadius = radius;
}

void CVehicleComponent::ReceiveData(NetPacket &packet)
{
	SetMaxEngineForce(packet->Read<Float>());
	SetMaxReverseEngineForce(packet->Read<Float>());
	SetMaxBrakeForce(packet->Read<Float>());
	SetAcceleration(packet->Read<Float>());
	SetTurnSpeed(packet->Read<Float>());
	SetMaxTurnAngle(packet->Read<Float>());
	SetFirstPersonCameraEnabled(packet->Read<bool>());
	SetThirdPersonCameraEnabled(packet->Read<bool>());
	SetBrakeForce(packet->Read<Float>());
	SetEngineForce(packet->Read<Float>());
	SetSteeringAngle(packet->Read<Float>());
	SetFrictionSlip(packet->Read<Float>());
	SetMaxSuspensionCompression(packet->Read<Float>());
	SetMaxSuspensionLength(packet->Read<Float>());
	SetRollInfluence(packet->Read<Float>());
	SetSuspensionStiffness(packet->Read<Float>());
	SetWheelDampingCompression(packet->Read<Float>());
	SetWheelDirection(packet->Read<Vector3>());
	SetWheelRadius(packet->Read<Float>());
	m_wheelInfo = packet->Read<WheelInfo>();
	m_hCbSteeringWheel = nwm::read_entity(packet,[this](BaseEntity *ent) {
		m_steeringWheel = ent->GetHandle();
		InitializeSteeringWheel();
	});

	auto numWheels = packet->Read<UChar>();
	for(UChar i=0;i<numWheels;i++)
		ReadWheelInfo(packet);
}
void CVehicleComponent::ClearDriver()
{
	if(m_playerAction.IsValid())
		m_playerAction.Remove();
	auto *entDriver = GetDriver();
	if(entDriver != nullptr && entDriver->IsPlayer())
	{
		auto plComponent = entDriver->GetPlayerComponent();
		if(plComponent->IsLocalPlayer())
		{
			c_game->EnableRenderMode(RenderMode::View);
			auto *vb = c_game->GetViewBody();
			if(vb != nullptr)
			{
				auto pRenderComponent = static_cast<CBaseEntity&>(vb->GetEntity()).GetRenderComponent();
				if(pRenderComponent.valid())
					pRenderComponent->SetRenderMode(RenderMode::View);
			}

			plComponent->SetObserverTarget(nullptr);
			plComponent->SetObserverCameraLocked(false);
			plComponent->SetObserverMode(OBSERVERMODE::FIRSTPERSON);
		}
	}
	BaseVehicleComponent::ClearDriver();
	if(m_camUpdate.IsValid())
		m_camUpdate.Remove();
}
void CVehicleComponent::SetCamera(bool bFirstPerson)
{
	auto *ent = GetDriver();
	if(!ent->IsPlayer() || !ent->GetPlayerComponent()->IsLocalPlayer() || m_bFirstPersonCamera == bFirstPerson)
		return;
	m_bFirstPersonCamera = bFirstPerson;
	auto &plComponent = ent->GetPlayerComponent();
	plComponent->SetObserverCameraLocked(bFirstPerson);
	plComponent->SetObserverMode(bFirstPerson ? OBSERVERMODE::FIRSTPERSON : OBSERVERMODE::THIRDPERSON);
}
void CVehicleComponent::ToggleCamera()
{
	auto *driver = GetDriver();
	if(driver == nullptr || !driver->IsPlayer())
		return;
	SetCamera(!m_bFirstPersonCamera);
}
void CVehicleComponent::SetDriver(BaseEntity *ent)
{
	if(ent == GetDriver())
		return;
	ClearDriver();
	CVehicleComponent::SetDriver(ent);
	if(!ent->IsPlayer() || !ent->GetPlayerComponent()->IsLocalPlayer())
		return;
	c_game->DisableRenderMode(RenderMode::View);
	auto *vb = c_game->GetViewBody();
	if(vb != nullptr)
	{
		auto pRenderComponent = static_cast<CBaseEntity&>(vb->GetEntity()).GetRenderComponent();
		if(pRenderComponent.valid())
			pRenderComponent->SetRenderMode(RenderMode::None);
	}
	auto plComponent = ent->GetPlayerComponent();
	plComponent->SetObserverMode(OBSERVERMODE::THIRDPERSON);
	auto pObsComponent = GetEntity().GetComponent<pragma::CObservableComponent>();
	plComponent->SetObserverTarget(pObsComponent.get());

	m_playerAction = plComponent->GetObserverModeProperty()->AddCallback([this,plComponent](const std::reference_wrapper<const OBSERVERMODE> oldObserverMode,const std::reference_wrapper<const OBSERVERMODE> observerMode) mutable {
		static auto bSkip = false;
		if(bSkip == true) // Prevent potential recursive loop
			return;
		bSkip = true;
		auto newObserverMode = observerMode.get();
		if(newObserverMode == OBSERVERMODE::FIRSTPERSON && IsFirstPersonCameraEnabled() == false)
		{
			newObserverMode = OBSERVERMODE::THIRDPERSON;
			plComponent->SetObserverMode(newObserverMode);
		}
		SetCamera(newObserverMode == OBSERVERMODE::FIRSTPERSON);
		bSkip = false;
	});
	FlagCallbackForRemoval(m_playerAction,CallbackType::Component);
	SetCamera(false);
}
void CVehicleComponent::Initialize()
{
	BaseVehicleComponent::Initialize();
}
Bool CVehicleComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	switch(eventId)
	{
		case NET_EVENT_VEHICLE_ADD_WHEEL:
		{
			ReadWheelInfo(packet);
			break;
		}
		case NET_EVENT_VEHICLE_SET_DRIVER:
		{
			auto *driver = nwm::read_entity(packet);
			if(driver == nullptr)
				ClearDriver();
			else
				SetDriver(driver);
			break;
		}
		case NET_EVENT_VEHICLE_MAX_ENGINE_FORCE:
		{
			auto force = packet->Read<Float>();
			SetMaxEngineForce(force);
			break;
		}
		case NET_EVENT_VEHICLE_MAX_REVERSE_ENGINE_FORCE:
		{
			auto force = packet->Read<Float>();
			SetMaxReverseEngineForce(force);
			break;
		}
		case NET_EVENT_VEHICLE_MAX_BRAKE_FORCE:
		{
			auto force = packet->Read<Float>();
			SetMaxBrakeForce(force);
			break;
		}
		case NET_EVENT_VEHICLE_ACCELERATION:
		{
			auto acc = packet->Read<Float>();
			SetAcceleration(acc);
			break;
		}
		case NET_EVENT_VEHICLE_TURN_SPEED:
		{
			auto speed = packet->Read<Float>();
			SetTurnSpeed(speed);
			break;
		}
		case NET_EVENT_VEHICLE_MAX_TURN_ANGLE:
		{
			auto ang = packet->Read<Float>();
			SetMaxTurnAngle(ang);
			break;
		}
		case NET_EVENT_VEHICLE_SET_FIRST_PERSON_CAMERA_ENABLED:
		{
			auto b = packet->Read<bool>();
			SetFirstPersonCameraEnabled(b);
			if(b == false && IsThirdPersonCameraEnabled() == true)
				SetCamera(false);
			break;
		}
		case NET_EVENT_VEHICLE_SET_THIRD_PERSON_CAMERA_ENABLED:
		{
			auto b = packet->Read<bool>();
			SetThirdPersonCameraEnabled(b);
			if(b == false && IsFirstPersonCameraEnabled() == true)
				SetCamera(true);
			break;
		}
		case NET_EVENT_VEHICLE_SET_STEERING_WHEEL:
		{
			auto *ent = nwm::read_entity(packet);
			m_steeringWheel = ent->GetHandle();
			InitializeSteeringWheel();
			break;
		}
		default:
			return false;
	}
	return true;
}
