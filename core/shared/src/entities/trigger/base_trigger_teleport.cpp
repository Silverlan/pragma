/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/trigger/base_trigger_teleport.hpp"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/trigger/base_trigger_touch.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

void BaseTriggerTeleportComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"target",false))
			m_target = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("model");
	ent.AddComponent("touch");
}

util::EventReply BaseTriggerTeleportComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseTouchComponent::EVENT_ON_START_TOUCH)
	{
		if(m_target.empty())
			return util::EventReply::Unhandled;
		auto &ent = GetEntity();
		auto *game = ent.GetNetworkState()->GetGameState();
		std::vector<BaseEntity*> targetCandidates;

		EntityIterator it {*game};
		it.AttachFilter<EntityIteratorFilterEntity>(m_target);
		for(auto *ent : it)
			targetCandidates.push_back(ent);

		if(targetCandidates.empty())
			return util::EventReply::Unhandled;
		auto *entTarget = targetCandidates[umath::random(0,targetCandidates.size() -1)];
		auto ptrTrComponent = ent.GetTransformComponent();
		auto ptrTrComponentTgt = entTarget->GetTransformComponent();
		if(ptrTrComponent && ptrTrComponentTgt)
		{
			ptrTrComponent->SetPosition(ptrTrComponentTgt->GetPosition());
			if(ent.GetSpawnFlags() &umath::to_integral(SpawnFlags::FaceTargetDirectionOnTeleport))
				ptrTrComponent->SetAngles(ptrTrComponentTgt->GetAngles());
		}
	}
	return util::EventReply::Unhandled;
}
