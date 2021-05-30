/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/point/constraints/point_constraint_base.h"
#include "pragma/physics/constraint.hpp"
#include <algorithm>
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseworld.h"
#include "pragma/physics/physobj.h"
#include <sharedutils/util_string.h>
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/components/base_name_component.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

void BasePointConstraintComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		return SetKeyValue(kvData.key,kvData.value) ? util::EventReply::Handled : util::EventReply::Unhandled;
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
	if(game.IsMapLoaded() == true)
	{
		InitializeConstraint();
		return;
	}
	m_cbGameLoaded = game.AddCallback("OnMapLoaded",FunctionCallback<void>::Create([this]() {
		InitializeConstraint();
	}));
}

void BasePointConstraintComponent::OnRemove()
{
	ClearConstraint();
	if(m_cbGameLoaded.IsValid())
		m_cbGameLoaded.Remove();
}

std::vector<util::TSharedHandle<physics::IConstraint>> &BasePointConstraintComponent::GetConstraints() {return m_constraints;}

util::EventReply BasePointConstraintComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseToggleComponent::EVENT_ON_TURN_ON)
		OnTurnOn();
	else if(eventId == BaseToggleComponent::EVENT_ON_TURN_OFF)
		OnTurnOff();
	return util::EventReply::Unhandled;
}

void BasePointConstraintComponent::OnTurnOn()
{
	for(unsigned int i=0;i<m_constraints.size();i++)
	{
		auto &hConstraint = m_constraints[i];
		if(hConstraint.IsValid())
			hConstraint->SetEnabled(true);
	}
}
void BasePointConstraintComponent::OnTurnOff()
{
	for(unsigned int i=0;i<m_constraints.size();i++)
	{
		auto &hConstraint = m_constraints[i];
		if(hConstraint.IsValid())
			hConstraint->SetEnabled(false);
	}
}

void BasePointConstraintComponent::InitializeConstraint()
{
	ClearConstraint();
	BaseEntity *entSrc = GetSourceEntity();
	if(entSrc == nullptr)
		return;
	std::vector<BaseEntity*> entsTgt;
	GetTargetEntities(entsTgt);
	auto pPhysComponentSrc = entSrc->GetPhysicsComponent();
	RigidPhysObj *physSrc = pPhysComponentSrc ? dynamic_cast<RigidPhysObj*>(pPhysComponentSrc->GetPhysicsObject()) : nullptr;
	if(physSrc == NULL)
		return;
	auto *bodySrc = physSrc->GetRigidBody();
	if(bodySrc == NULL)
		return;
	for(unsigned int i=0;i<entsTgt.size();i++)
		InitializeConstraint(entSrc,entsTgt[i]);
	BaseEntity &entThis = GetEntity();
	unsigned int flags = entThis.GetSpawnFlags();
	if(flags &SF_CONSTRAINT_START_INACTIVE)
	{
		auto *pToggleComponent = static_cast<pragma::BaseToggleComponent*>(GetEntity().FindComponent("toggle").get());
		if(pToggleComponent != nullptr)
			pToggleComponent->TurnOff();
	}
	if(flags &SF_CONSTRAINT_DISABLE_COLLISIONS)
	{
		for(auto it=m_constraints.begin();it!=m_constraints.end();++it)
		{
			auto &hConstraint = *it;
			if(hConstraint.IsValid())
				hConstraint->DisableCollisions();
		}
	}
}


void BasePointConstraintComponent::ClearConstraint()
{
	for(unsigned int i=0;i<m_constraints.size();i++)
	{
		auto &c = m_constraints[i];
		if(c.IsValid())
			c.Remove();
	}
	m_constraints.clear();
}

bool BasePointConstraintComponent::SetKeyValue(std::string key,std::string val)
{
	if(key == "source")
	{
		m_kvSource = val;
		ustring::to_lower(m_kvSource);
	}
	else if(key == "target")
	{
		m_kvTarget = val;
		ustring::to_lower(m_kvTarget);
	}
	else if(key == "target_origin")
		m_posTarget = uvec::create(val);
	else
		return false;
	return true;
}

void BasePointConstraintComponent::GetTargetEntities(std::vector<BaseEntity*> &entsTgt)
{
	BaseEntity &ent = GetEntity();
	NetworkState *state = ent.GetNetworkState();
	Game *game = state->GetGameState();
	if(m_kvTarget.empty())
		return;
	else
	{
		EntityIterator it {*game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
		it.AttachFilter<EntityIteratorFilterComponent>("physics");
		it.AttachFilter<EntityIteratorFilterEntity>(m_kvTarget);
		for(auto *ent : it)
			entsTgt.push_back(ent);
	}
}

BaseEntity *BasePointConstraintComponent::GetSourceEntity()
{
	BaseEntity &entThis = GetEntity();
	NetworkState *state = entThis.GetNetworkState();
	Game *game = state->GetGameState();
	if(m_kvSource.empty())
	{
		auto *pWorld = game->GetWorld();
		return (pWorld != nullptr) ? &pWorld->GetEntity() : nullptr;
	}

	EntityIterator entIt {*game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<EntityIteratorFilterComponent>("physics");
	entIt.AttachFilter<EntityIteratorFilterEntity>(m_kvSource);
	auto it = entIt.begin();
	return (it != entIt.end()) ? *it : nullptr;
}

void BasePointConstraintComponent::InitializeConstraint(BaseEntity*,BaseEntity*) {}
