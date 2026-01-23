// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.vehicle;
import :entities.components.observable;
import :entities.components.observer;
import :entities.components.render;
import :entities.components.view_body;
import :game;

using namespace pragma;

std::vector<CVehicleComponent *> CVehicleComponent::s_vehicles;
const std::vector<CVehicleComponent *> &CVehicleComponent::GetAll() { return s_vehicles; }
unsigned int CVehicleComponent::GetVehicleCount() { return CUInt32(s_vehicles.size()); }

CVehicleComponent::CVehicleComponent(ecs::BaseEntity &ent) : BaseVehicleComponent(ent), CBaseSnapshotComponent {} { s_vehicles.push_back(this); }

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
	auto gear = packet->Read<physics::IVehicle::Gear>();
	auto brakeFactor = packet->Read<float>();
	auto handbrakeFactor = packet->Read<float>();
	auto accFactor = packet->Read<float>();
	auto engineRotSpeed = packet->Read<math::Radian>();
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

void CVehicleComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

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
	auto *entSteeringWheel = networking::read_entity(packet);
	m_steeringWheel = entSteeringWheel ? entSteeringWheel->GetHandle() : EntityHandle {};

	auto *driver = networking::read_entity(packet);
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
			get_cgame()->EnableRenderMode(rendering::SceneRenderPass::View);
			auto *vb = get_cgame()->GetViewBody<CViewBodyComponent>();
			if(vb != nullptr) {
				auto pRenderComponent = static_cast<ecs::CBaseEntity &>(vb->GetEntity()).GetRenderComponent();
				if(pRenderComponent)
					pRenderComponent->SetSceneRenderPass(rendering::SceneRenderPass::View);
			}

			auto *cam = get_cgame()->GetPrimaryCamera<CCameraComponent>();
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
void CVehicleComponent::SetDriver(ecs::BaseEntity *ent)
{
	if(ent == GetDriver())
		return;
	ClearDriver();
	BaseVehicleComponent::SetDriver(ent);
	if(!ent->IsPlayer() || !ent->GetPlayerComponent()->IsLocalPlayer())
		return;
	get_cgame()->DisableRenderMode(rendering::SceneRenderPass::View);
	auto *vb = get_cgame()->GetViewBody<CViewBodyComponent>();
	if(vb != nullptr) {
		auto pRenderComponent = static_cast<ecs::CBaseEntity &>(vb->GetEntity()).GetRenderComponent();
		if(pRenderComponent)
			pRenderComponent->SetSceneRenderPass(rendering::SceneRenderPass::None);
	}
	auto plComponent = ent->GetPlayerComponent();
	if(plComponent.expired())
		return;
	auto *observable = plComponent->GetObservableComponent();
	auto *observer = observable ? observable->GetObserver() : nullptr;
	if(!observer)
		return;
	observer->SetObserverMode(ObserverMode::ThirdPerson);

	auto vhcObservableC = GetEntity().GetComponent<CObservableComponent>();
	observer->SetObserverTarget(vhcObservableC.get());
}
void CVehicleComponent::Initialize() { BaseVehicleComponent::Initialize(); }
Bool CVehicleComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSteeringWheelModel) {
		auto *ent = networking::read_entity(packet);
		m_steeringWheel = ent ? ent->GetHandle() : EntityHandle {};
		m_maxSteeringWheelAngle = packet->Read<float>();
		InitializeSteeringWheel();
	}
	else if(eventId == m_netEvSetDriver) {
		auto *ent = networking::read_entity(packet);
		if(ent)
			SetDriver(ent);
		else
			ClearDriver();
	}
	else
		return false;
	return true;
}

void CVehicleComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BaseVehicleComponent::RegisterLuaBindings(l, modEnts);
	auto def = pragma::LuaCore::create_entity_component_class<CVehicleComponent, BaseVehicleComponent>("VehicleComponent");
	modEnts[def];
}
