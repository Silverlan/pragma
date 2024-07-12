/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
#include "pragma/entities/components/c_observer_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/input/inkeys.h>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

std::vector<CVehicleComponent *> CVehicleComponent::s_vehicles;
const std::vector<CVehicleComponent *> &CVehicleComponent::GetAll() { return s_vehicles; }
unsigned int CVehicleComponent::GetVehicleCount() { return CUInt32(s_vehicles.size()); }

CVehicleComponent::CVehicleComponent(BaseEntity &ent) : BaseVehicleComponent(ent), CBaseSnapshotComponent {} { s_vehicles.push_back(this); }

CVehicleComponent::~CVehicleComponent()
{
	if(m_hCbSteeringWheel.IsValid())
		m_hCbSteeringWheel.Remove();
	ClearDriver();
	for(int i = 0; i < s_vehicles.size(); i++) {
		if(s_vehicles[i] == this) {
			s_vehicles.erase(s_vehicles.begin() + i);
			break;
		}
	}
}

void CVehicleComponent::ReceiveSnapshotData(NetPacket &packet)
{
	auto *physVehicle = GetPhysicsVehicle();
	auto steerFactor = packet->Read<float>();
	auto gear = packet->Read<pragma::physics::IVehicle::Gear>();
	auto brakeFactor = packet->Read<float>();
	auto handbrakeFactor = packet->Read<float>();
	auto accFactor = packet->Read<float>();
	auto engineRotSpeed = packet->Read<umath::Radian>();
	if(physVehicle) {
		physVehicle->SetSteerFactor(steerFactor);
		physVehicle->SetGear(gear);
		physVehicle->SetBrakeFactor(brakeFactor);
		physVehicle->SetHandbrakeFactor(handbrakeFactor);
		physVehicle->SetAccelerationFactor(accFactor);
		physVehicle->SetEngineRotationSpeed(engineRotSpeed);
		auto numWheels = GetWheelCount();
		for(auto i = decltype(numWheels) {0u}; i < numWheels; ++i)
			physVehicle->SetWheelRotationSpeed(i, packet->Read<float>());
	}
}

void CVehicleComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

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
	//if(AddWheel(conPoint,axle,bFrontWheel,&wheelId,translation,rotation) == false)
	//	return;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *info = GetWheelInfo(wheelId);
	if(info == nullptr)
		return;
	info->m_wheelDirectionCS = btVector3(wheelDir.x, wheelDir.y, wheelDir.z);
	info->m_suspensionRestLength1 = suspensionRest;
	info->m_wheelsRadius = radius;
#endif
}

void CVehicleComponent::ReceiveData(NetPacket &packet)
{
	auto *entSteeringWheel = nwm::read_entity(packet);
	m_steeringWheel = entSteeringWheel ? entSteeringWheel->GetHandle() : EntityHandle {};

	auto *driver = nwm::read_entity(packet);
	if(driver)
		SetDriver(driver);
	else
		ClearDriver();
}

void CVehicleComponent::OnEntitySpawn()
{
	BaseVehicleComponent::OnEntitySpawn();
	InitializeSteeringWheel();
}

void CVehicleComponent::ClearDriver()
{
	auto *entDriver = GetDriver();
	if(entDriver != nullptr && entDriver->IsPlayer()) {
		auto plComponent = entDriver->GetPlayerComponent();
		if(plComponent->IsLocalPlayer()) {
			c_game->EnableRenderMode(pragma::rendering::SceneRenderPass::View);
			auto *vb = c_game->GetViewBody();
			if(vb != nullptr) {
				auto pRenderComponent = static_cast<CBaseEntity &>(vb->GetEntity()).GetRenderComponent();
				if(pRenderComponent)
					pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::View);
			}

			auto *cam = c_game->GetPrimaryCamera();
			if(cam) {
				auto observerC = cam->GetEntity().GetComponent<CObserverComponent>();
				if(observerC.valid()) {
					observerC->SetObserverTarget(nullptr);
					observerC->SetObserverMode(ObserverMode::FirstPerson);
				}
			}
		}
	}
	BaseVehicleComponent::ClearDriver();
}
void CVehicleComponent::SetDriver(BaseEntity *ent)
{
	if(ent == GetDriver())
		return;
	ClearDriver();
	BaseVehicleComponent::SetDriver(ent);
	if(!ent->IsPlayer() || !ent->GetPlayerComponent()->IsLocalPlayer())
		return;
	c_game->DisableRenderMode(pragma::rendering::SceneRenderPass::View);
	auto *vb = c_game->GetViewBody();
	if(vb != nullptr) {
		auto pRenderComponent = static_cast<CBaseEntity &>(vb->GetEntity()).GetRenderComponent();
		if(pRenderComponent)
			pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::None);
	}
	auto plComponent = ent->GetPlayerComponent();
	if(plComponent.expired())
		return;
	auto *observable = plComponent->GetObservableComponent();
	auto *observer = observable ? observable->GetObserver() : nullptr;
	if(!observer)
		return;
	observer->SetObserverMode(ObserverMode::ThirdPerson);

	auto vhcObservableC = GetEntity().GetComponent<pragma::CObservableComponent>();
	observer->SetObserverTarget(vhcObservableC.get());
}
void CVehicleComponent::Initialize() { BaseVehicleComponent::Initialize(); }
Bool CVehicleComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSteeringWheelModel) {
		auto *ent = nwm::read_entity(packet);
		m_steeringWheel = ent ? ent->GetHandle() : EntityHandle {};
		m_maxSteeringWheelAngle = packet->Read<float>();
		InitializeSteeringWheel();
	}
	else if(eventId == m_netEvSetDriver) {
		auto *ent = nwm::read_entity(packet);
		if(ent)
			SetDriver(ent);
		else
			ClearDriver();
	}
	else
		return false;
	return true;
}
