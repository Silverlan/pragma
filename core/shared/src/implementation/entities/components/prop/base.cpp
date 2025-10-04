// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "mathutil/umath.h"

#include "sharedutils/util.h"

#include "sharedutils/util_string.h"

#include "mathutil/uvec.h"

module pragma.shared;

import :entities.components.props.base;

using namespace pragma;

BasePropComponent::BasePropComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

PHYSICSTYPE BasePropComponent::UpdatePhysicsType(BaseEntity *ent)
{
	if(ent->GetSpawnFlags() & umath::to_integral(SpawnFlags::Static)) {
		m_kvMass = 0.f;
		return PHYSICSTYPE::STATIC;
	}
	auto &hMdl = ent->GetModel();
	if(hMdl != nullptr) {
		// TODO: Do this in a better way
		for(auto &colMesh : hMdl->GetCollisionMeshes()) {
			if(colMesh->IsSoftBody())
				return PHYSICSTYPE::SOFTBODY;
		}
	}
	return PHYSICSTYPE::DYNAMIC;
}

bool BasePropComponent::SetKeyValue(std::string key, std::string val)
{
	if(key == "scale") {
		Vector3 scale {1.f, 1.f, 1.f};
		auto n = ustring::string_to_array<float, double>(val, &scale.x, atof, 3);
		if(n == 1) {
			scale.y = scale.x;
			scale.z = scale.x;
		}
		m_kvScale = scale;
	}
	else if(key == "mass") {
		ustring::remove_whitespace(val);
		if(val.empty() == false)
			m_kvMass = ustring::to_float(val);
	}
	else
		return false;
	return true;
}

void BasePropComponent::InitializePhysics(PHYSICSTYPE physType)
{
	auto &ent = GetEntity();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr)
		return;
	if((ent.GetSpawnFlags() & umath::to_integral(SpawnFlags::DisableCollisions)) != 0 || physType == PHYSICSTYPE::NONE)
		return;
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	auto *phys = pPhysComponent->InitializePhysics(physType);
	if(phys != nullptr) {
		phys->WakeUp();
		if(std::isnan(m_kvMass) == false)
			phys->SetMass(m_kvMass);
	}
}

void BasePropComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		return SetKeyValue(kvData.key, kvData.value) ? util::EventReply::Handled : util::EventReply::Unhandled;
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto physComponent = GetEntity().GetPhysicsComponent();
		if(!physComponent || physComponent->GetJoints().empty() == true)
			return;
		// We only need an animated component if this is a ragdoll (i.e. the physics component has joints)
		GetEntity().AddComponent("animated");
	});

	auto &ent = GetEntity();
	ent.AddComponent("model");
	ent.AddComponent("physics");
	ent.AddComponent("flammable");
	ent.AddComponent("sound_emitter");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent *>(whRenderComponent.get())->SetCastShadows(true);

	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { InitializePhysics(); });
}

void BasePropComponent::InitializePhysics()
{
	auto &ent = GetEntity();
	if(ent.IsSpawned() == false)
		return;
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	auto &hMdl = ent.GetModel();
	if(hMdl != nullptr && m_physicsType != PHYSICSTYPE::NONE && m_physicsType != pPhysComponent->GetPhysicsType())
		InitializePhysics(m_physicsType);
	if(m_moveType != MOVETYPE::NONE)
		pPhysComponent->SetMoveType(m_moveType);
}

void BasePropComponent::Setup(PHYSICSTYPE physType, MOVETYPE mvType)
{
	m_physicsType = physType;
	m_moveType = mvType;
}

void BasePropComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	if(m_kvScale != Vector3 {1.f, 1.f, 1.f}) {
		auto pTrComponent = ent.GetTransformComponent();
		if(pTrComponent != nullptr)
			pTrComponent->SetScale(m_kvScale);
	}
	InitializePhysics();
}
