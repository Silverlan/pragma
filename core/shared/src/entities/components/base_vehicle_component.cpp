/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_vehicle_component.hpp"
#include "pragma/entities/components/base_wheel_component.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/input/inkeys.h"
#include "pragma/model/model.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/physics/shape.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_attachment_component.hpp"
#include "pragma/entities/components/action_input_controller_component.hpp"
#include "pragma/physics/raytraces.h"
#include "pragma/util/util_game.hpp"
#include "pragma/logging.hpp"

//#define ENABLE_DEPRECATED_PHYSICS

using namespace pragma;

ComponentEventId BaseVehicleComponent::EVENT_ON_DRIVER_ENTERED = pragma::INVALID_COMPONENT_ID;
ComponentEventId BaseVehicleComponent::EVENT_ON_DRIVER_EXITED = pragma::INVALID_COMPONENT_ID;
void BaseVehicleComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_DRIVER_ENTERED = registerEvent("ON_DRIVER_ENTERED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_DRIVER_EXITED = registerEvent("ON_DRIVER_EXITED", ComponentEventInfo::Type::Broadcast);
}
BaseVehicleComponent::BaseVehicleComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseVehicleComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_physVehicle.IsValid())
		m_physVehicle.Remove();
	for(auto &wheel : m_wheels) {
		if(wheel.hWheel.expired())
			continue;
		wheel.hWheel->GetEntity().Remove();
	}
}

void BaseVehicleComponent::InitializeVehiclePhysics(PHYSICSTYPE type, BasePhysicsComponent::PhysFlags flags)
{
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return;
	auto &mdl = ent.GetModel();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(mdl == nullptr || !pPhysComponent)
		return;
	auto &colMeshes = mdl->GetCollisionMeshes();
	if(colMeshes.empty())
		return;
	std::vector<pragma::physics::IShape *> chassisShapes {};
	chassisShapes.reserve(colMeshes.size());
	for(auto &colMesh : colMeshes)
		chassisShapes.push_back(colMesh->GetShape().get());

	auto chassisMesh = physEnv->CreateCompoundShape(chassisShapes);

	physics::PhysObjCreateInfo physObjCreateInfo {};
	auto numChassisMeshes = physObjCreateInfo.AddShape(*chassisMesh);
	m_vhcCreateInfo.chassis.shapeIndices.clear();
	m_vhcCreateInfo.chassis.shapeIndices.reserve(numChassisMeshes);
	for(uint32_t i = 0u; i < numChassisMeshes; ++i)
		m_vhcCreateInfo.chassis.shapeIndices.push_back(i);

	auto wheelMeshOffset = numChassisMeshes;
	for(auto wheelIndex = decltype(m_wheels.size()) {0u}; wheelIndex < m_wheels.size(); ++wheelIndex) {
		auto &wheel = m_wheels.at(wheelIndex);
		auto mdl = game->LoadModel(wheel.model);
		if(mdl == nullptr)
			continue;
		auto &colMeshes = mdl->GetCollisionMeshes();
		auto colMesh = (colMeshes.empty() == false) ? colMeshes.front() : nullptr;
		auto shape = colMesh ? colMesh->GetShape() : nullptr;
		if(shape == nullptr)
			continue;
		if(shape->IsCompoundShape()) {
			spdlog::warn("Vehicle wheel is a compound shape, which is not supported for wheels! Skipping...");
			continue;
		}
		m_vhcCreateInfo.wheels.at(wheelIndex).shapeIndex = wheelMeshOffset;
		wheelMeshOffset += physObjCreateInfo.AddShape(*shape);
	}

	auto physObj = pPhysComponent->InitializePhysics(physObjCreateInfo, flags);
	if(physObj.expired())
		return;
	auto *colObj = physObj->GetCollisionObject();
	if(colObj == nullptr || colObj->IsRigid() == false)
		return;
	auto *pRigidBody = colObj->GetRigidBody();

	m_vhcCreateInfo.actor = util::shared_handle_cast<physics::IBase, physics::IRigidBody>(pRigidBody->ClaimOwnership());
	pRigidBody->SetCollisionFilterGroup(CollisionMask::Vehicle);
	pRigidBody->SetCollisionFilterMask(CollisionMask::All);

	auto vhc = physEnv->CreateVehicle(m_vhcCreateInfo);
	vhc->SetGear(physics::IVehicle::Gear::First);
	vhc->SetUseAutoGears(true);
	vhc->SetBrakeFactor(1.0);
	vhc->SetUseDigitalInputs(true);
	vhc->Spawn();
	m_physVehicle = vhc;

	InitializeWheelEntities();
	if(m_steeringWheelMdl.empty() == false)
		SetupSteeringWheel(m_steeringWheelMdl, m_maxSteeringWheelAngle);
}

void BaseVehicleComponent::DestroyVehiclePhysics()
{
	if(m_physVehicle.IsValid())
		m_physVehicle.Remove();
}

physics::IVehicle *BaseVehicleComponent::GetPhysicsVehicle() { return m_physVehicle.Get(); }
const physics::IVehicle *BaseVehicleComponent::GetPhysicsVehicle() const { return const_cast<BaseVehicleComponent *>(this)->GetPhysicsVehicle(); }

float BaseVehicleComponent::GetSpeedKmh() const
{
	if(m_physVehicle == nullptr)
		return 0.f;
	return pragma::units_to_metres(m_physVehicle->GetForwardSpeed()) / 1'000.f * 60.f * 60.f;
}

void BaseVehicleComponent::InitializeSteeringWheel()
{
	auto *ent = GetSteeringWheel();
	if(ent == nullptr) // || umath::is_flag_set(m_stateFlags,StateFlags::SteeringWheelInitialized))
		return;
	umath::set_flag(m_stateFlags, StateFlags::SteeringWheelInitialized);
	if(m_cbSteeringWheel.IsValid())
		m_cbSteeringWheel.Remove();
	auto pAttachableComponent = ent->AddComponent("attachment");
	if(pAttachableComponent.expired())
		return;
	AttachmentInfo attInfo {};
	attInfo.flags |= FAttachmentMode::SnapToOrigin | FAttachmentMode::UpdateEachFrame;
	static_cast<BaseAttachmentComponent *>(pAttachableComponent.get())->AttachToAttachment(&GetEntity(), "steering_wheel", attInfo);

	m_cbSteeringWheel = pAttachableComponent->AddEventCallback(BaseAttachmentComponent::EVENT_ON_ATTACHMENT_UPDATE, [this, pAttachableComponent](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto pTrComponentSteeringWheel = pAttachableComponent->GetEntity().GetTransformComponent();
		if(!pTrComponentSteeringWheel)
			return util::EventReply::Unhandled;
		auto ang = EulerAngles(-GetSteeringFactor() * m_maxSteeringWheelAngle, 0.f, 0.f);
		auto rot = uquat::create(ang);
		auto rotEnt = pTrComponentSteeringWheel->GetRotation();
		rotEnt = rotEnt * rot;
		pTrComponentSteeringWheel->SetRotation(rotEnt);
		return util::EventReply::Unhandled;
	});
}

void BaseVehicleComponent::SetupSteeringWheel(const std::string &mdl, umath::Degree maxSteeringAngle)
{
	m_steeringWheelMdl = mdl;
	m_maxSteeringWheelAngle = maxSteeringAngle;
	auto &entThis = GetEntity();
	if(!entThis.IsSpawned())
		return;
	if(!m_steeringWheel.valid()) {
		auto *ent = entThis.GetNetworkState()->GetGameState()->CreateEntity("prop_dynamic");
		if(ent == nullptr)
			return;
		m_steeringWheel = ent->GetHandle();
		ent->Spawn();
		entThis.RemoveEntityOnRemoval(ent);
	}
	auto mdlComponent = m_steeringWheel->GetModelComponent();
	if(!mdlComponent)
		return;
	mdlComponent->SetModel(mdl.c_str());
	InitializeSteeringWheel();
}

void BaseVehicleComponent::ClearDriver()
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::HasDriver) == false)
		return;
	BroadcastEvent(EVENT_ON_DRIVER_EXITED);
	umath::set_flag(m_stateFlags, StateFlags::HasDriver, false);
	SetTickPolicy(TickPolicy::Never);
	m_driver = EntityHandle();
	if(m_physVehicle.IsValid())
		m_physVehicle->ResetControls();
}

void BaseVehicleComponent::SetDriver(BaseEntity *ent)
{
	if(m_driver.valid())
		ClearDriver();
	m_driver = ent->GetHandle();
	umath::set_flag(m_stateFlags, StateFlags::HasDriver, true);
	SetTickPolicy(TickPolicy::Always);
	BroadcastEvent(EVENT_ON_DRIVER_ENTERED);
}

BaseEntity *BaseVehicleComponent::GetDriver() { return m_driver.get(); }
Bool BaseVehicleComponent::HasDriver() const { return m_driver.valid(); }

BaseWheelComponent *BaseVehicleComponent::CreateWheelEntity(uint8_t wheelIndex)
{
	auto *entWheel = GetEntity().GetNetworkState()->GetGameState()->CreateEntity("vhc_wheel");
	if(entWheel == nullptr)
		return nullptr;
	auto wheelComponent = entWheel->FindComponent("wheel");
	if(wheelComponent.expired())
		return nullptr;
	auto &hWheel = m_wheels.at(wheelIndex).hWheel;
	hWheel = m_wheels.at(wheelIndex).hWheel = wheelComponent->GetHandle<BaseWheelComponent>();
	return hWheel.get();
}

void BaseVehicleComponent::InitializeWheelEntities()
{
	auto numWheels = umath::min(m_wheels.size(), m_vhcCreateInfo.wheels.size());
	for(auto i = decltype(numWheels) {0u}; i < numWheels; ++i) {
		auto &wheelData = m_wheels.at(i);
		auto &wheelDesc = m_vhcCreateInfo.wheels.at(i);
		if(wheelData.model.empty())
			continue; // Wheel has no model, no need to create an entity for it
		auto *wheel = CreateWheelEntity(i);
		if(wheel == nullptr)
			continue;
		auto mdlComponent = wheel->GetEntity().GetModelComponent();
		if(mdlComponent)
			mdlComponent->SetModel(wheelData.model);
		wheel->SetupWheel(*this, wheelDesc, i);
		wheel->GetEntity().Spawn();
		wheelData.hWheel = wheel->GetHandle<BaseWheelComponent>();
	}
}

BaseEntity *BaseVehicleComponent::GetSteeringWheel() { return m_steeringWheel.get(); }

void BaseVehicleComponent::SetupVehicle(const pragma::physics::VehicleCreateInfo &createInfo, const std::vector<std::string> &wheelModels)
{
	m_vhcCreateInfo = createInfo;
	auto numWheels = createInfo.wheels.size();
	m_wheels.resize(numWheels);
	uint32_t wheelIndex = 0u;
	for(auto &wheel : m_wheels) {
		wheel.model = (wheelIndex < wheelModels.size()) ? wheelModels.at(wheelIndex) : ((wheelModels.empty() == false) ? wheelModels.back() : "");
		++wheelIndex;
	}
}

void BaseVehicleComponent::OnTick(double tDelta)
{
	auto *driver = GetDriver();
	if(m_physVehicle == nullptr || umath::is_flag_set(m_stateFlags, StateFlags::HasDriver) == false)
		return;
	if(driver == nullptr || driver->IsPlayer() == false) {
		ClearDriver();
		return;
	}
	auto plComponent = driver->GetPlayerComponent();

	auto accFactor = 0.f;
	auto *actionC = plComponent->GetActionInputController();
	if(actionC && actionC->GetActionInput(Action::MoveForward))
		accFactor += 1.f;
	if(actionC && actionC->GetActionInput(Action::MoveBackward))
		accFactor -= 1.f;
	m_physVehicle->SetAccelerationFactor(accFactor);

	auto brakeFactor = (actionC && actionC->GetActionInput(Action::Jump)) ? 1.f : 0.f;
	m_physVehicle->SetBrakeFactor(brakeFactor);

	auto steerFactor = 0.f;
	if(actionC && actionC->GetActionInput(Action::MoveLeft))
		steerFactor -= 1.f;
	if(actionC && actionC->GetActionInput(Action::MoveRight))
		steerFactor += 1.f;
	m_physVehicle->SetSteerFactor(steerFactor);
}
void BaseVehicleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BasePhysicsComponent::EVENT_INITIALIZE_PHYSICS, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto pPhysComponent = GetEntity().GetPhysicsComponent();
		if(!pPhysComponent)
			return util::EventReply::Unhandled;
		auto &physInitData = static_cast<CEInitializePhysics &>(evData.get());
		if(physInitData.physicsType != PHYSICSTYPE::DYNAMIC)
			return util::EventReply::Unhandled;
		InitializeVehiclePhysics(physInitData.physicsType, physInitData.flags);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { DestroyVehiclePhysics(); });

	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter() | CollisionMask::Vehicle);
	ent.AddComponent("model");
	ent.AddComponent("physics");
	ent.AddComponent("observable");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent *>(whRenderComponent.get())->SetCastShadows(true);
	m_netEvSteeringWheelModel = SetupNetEvent("steering_wheel");
	m_netEvSetDriver = SetupNetEvent("set_driver");
}

uint8_t BaseVehicleComponent::GetWheelCount() const { return static_cast<uint8_t>(m_wheels.size()); }

Float BaseVehicleComponent::GetSteeringFactor() const
{
	if(m_physVehicle == nullptr || m_vhcCreateInfo.wheels.empty())
		return 0.f;
	return umath::clamp(static_cast<float>(umath::rad_to_deg(m_physVehicle->GetWheelYawAngle(0)) / m_vhcCreateInfo.wheels.front().maxSteeringAngle), -1.f, 1.f);
}
