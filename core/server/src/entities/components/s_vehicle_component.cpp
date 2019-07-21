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
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/physics/environment.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

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
	// TODO
	//NetPacket p;
	//nwm::write_entity(p,nullptr);
	//static_cast<SBaseEntity&>(GetEntity()).SendNetEvent(NET_EVENT_VEHICLE_SET_DRIVER,p,pragma::networking::Protocol::SlowReliable);
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
	// TODO
	//NetPacket p;
	//nwm::write_entity(p,ent);
	//entThis.SendNetEvent(NET_EVENT_VEHICLE_SET_DRIVER,p,pragma::networking::Protocol::SlowReliable);
}

#ifdef ENABLE_DEPRECATED_PHYSICS
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
#endif

void SVehicleComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	nwm::write_entity(packet,m_steeringWheel);
}

void SVehicleComponent::SetSteeringWheelModel(const std::string &mdl)
{
	BaseVehicleComponent::SetSteeringWheelModel(mdl);
	NetPacket p;
	nwm::write_entity(p,m_steeringWheel);
	static_cast<SBaseEntity&>(GetEntity()).SendNetEvent(m_netEvSteeringWheelModel,p,pragma::networking::Protocol::SlowReliable);
}

BaseWheelComponent *SVehicleComponent::CreateWheelEntity(uint8_t wheelIndex)
{
	auto *wheel = BaseVehicleComponent::CreateWheelEntity(wheelIndex);
	if(wheel == nullptr)
		return nullptr;
	static_cast<SBaseEntity&>(wheel->GetEntity()).SetSynchronized(false);
	return wheel;
}

void SVehicleComponent::OnPostSpawn()
{
	if(!m_steeringWheelMdl.empty())
	{
		SetSteeringWheelModel(m_steeringWheelMdl);
		m_steeringWheelMdl.clear();
	}
}
