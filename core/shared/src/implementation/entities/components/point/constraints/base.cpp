// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.point.constraints.base;

using namespace pragma;

void BasePointConstraintComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		return SetKeyValue(kvData.key, kvData.value) ? util::EventReply::Handled : util::EventReply::Unhandled;
	});
	auto &ent = GetEntity();
	ent.AddComponent("toggle");
	ent.AddComponent("transform");
	ent.AddComponent("name");
}

void BasePointConstraintComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	if(game.IsMapLoaded() == true) {
		InitializeConstraint();
		return;
	}
	m_cbGameLoaded = game.AddCallback("OnMapLoaded", FunctionCallback<void>::Create([this]() { InitializeConstraint(); }));
}

void BasePointConstraintComponent::OnRemove()
{
	ClearConstraint();
	if(m_cbGameLoaded.IsValid())
		m_cbGameLoaded.Remove();
}

std::vector<util::TSharedHandle<physics::IConstraint>> &BasePointConstraintComponent::GetConstraints() { return m_constraints; }

util::EventReply BasePointConstraintComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseToggleComponent::EVENT_ON_TURN_ON)
		OnTurnOn();
	else if(eventId == baseToggleComponent::EVENT_ON_TURN_OFF)
		OnTurnOff();
	return util::EventReply::Unhandled;
}

void BasePointConstraintComponent::OnTurnOn()
{
	for(unsigned int i = 0; i < m_constraints.size(); i++) {
		auto &hConstraint = m_constraints[i];
		if(hConstraint.IsValid())
			hConstraint->SetEnabled(true);
	}
}
void BasePointConstraintComponent::OnTurnOff()
{
	for(unsigned int i = 0; i < m_constraints.size(); i++) {
		auto &hConstraint = m_constraints[i];
		if(hConstraint.IsValid())
			hConstraint->SetEnabled(false);
	}
}

void BasePointConstraintComponent::InitializeConstraint()
{
	ClearConstraint();
	ecs::BaseEntity *entSrc = GetSourceEntity();
	if(entSrc == nullptr)
		return;
	std::vector<ecs::BaseEntity *> entsTgt;
	GetTargetEntities(entsTgt);
	auto pPhysComponentSrc = entSrc->GetPhysicsComponent();
	auto *physSrc = pPhysComponentSrc ? dynamic_cast<physics::RigidPhysObj *>(pPhysComponentSrc->GetPhysicsObject()) : nullptr;
	if(physSrc == nullptr)
		return;
	auto *bodySrc = physSrc->GetRigidBody();
	if(bodySrc == nullptr)
		return;
	for(unsigned int i = 0; i < entsTgt.size(); i++)
		InitializeConstraint(entSrc, entsTgt[i]);
	ecs::BaseEntity &entThis = GetEntity();
	unsigned int flags = entThis.GetSpawnFlags();
	if(flags & SF_CONSTRAINT_START_INACTIVE) {
		auto *pToggleComponent = static_cast<BaseToggleComponent *>(GetEntity().FindComponent("toggle").get());
		if(pToggleComponent != nullptr)
			pToggleComponent->TurnOff();
	}
	if(flags & SF_CONSTRAINT_DISABLE_COLLISIONS) {
		for(auto it = m_constraints.begin(); it != m_constraints.end(); ++it) {
			auto &hConstraint = *it;
			if(hConstraint.IsValid())
				hConstraint->DisableCollisions();
		}
	}
}

void BasePointConstraintComponent::ClearConstraint()
{
	for(unsigned int i = 0; i < m_constraints.size(); i++) {
		auto &c = m_constraints[i];
		if(c.IsValid())
			c.Remove();
	}
	m_constraints.clear();
}

bool BasePointConstraintComponent::SetKeyValue(std::string key, std::string val)
{
	if(key == "source") {
		m_kvSource = val;
		string::to_lower(m_kvSource);
	}
	else if(key == "target") {
		m_kvTarget = val;
		string::to_lower(m_kvTarget);
	}
	else if(key == "target_origin")
		m_posTarget = uvec::create(val);
	else
		return false;
	return true;
}

void BasePointConstraintComponent::GetTargetEntities(std::vector<ecs::BaseEntity *> &entsTgt)
{
	ecs::BaseEntity &ent = GetEntity();
	auto *state = ent.GetNetworkState();
	Game *game = state->GetGameState();
	if(m_kvTarget.empty())
		return;
	else {
		ecs::EntityIterator it {*game, ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
		it.AttachFilter<EntityIteratorFilterComponent>("physics");
		it.AttachFilter<EntityIteratorFilterEntity>(m_kvTarget);
		for(auto *ent : it)
			entsTgt.push_back(ent);
	}
}

ecs::BaseEntity *BasePointConstraintComponent::GetSourceEntity()
{
	ecs::BaseEntity &entThis = GetEntity();
	auto *state = entThis.GetNetworkState();
	Game *game = state->GetGameState();
	if(m_kvSource.empty()) {
		auto *pWorld = game->GetWorld();
		return (pWorld != nullptr) ? &pWorld->GetEntity() : nullptr;
	}

	ecs::EntityIterator entIt {*game, ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<EntityIteratorFilterComponent>("physics");
	entIt.AttachFilter<EntityIteratorFilterEntity>(m_kvSource);
	auto it = entIt.begin();
	return (it != entIt.end()) ? *it : nullptr;
}

void BasePointConstraintComponent::InitializeConstraint(ecs::BaseEntity *, ecs::BaseEntity *) {}
