/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/components/basepointpathnode.h"
#include <sharedutils/util_string.h>
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

void BasePointPathNodeComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "next_node", false))
			m_kvNextNode = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void BasePointPathNodeComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(!m_kvNextNode.empty()) {
		auto componentId = GetComponentId();
		EntityIterator entIterator {*GetEntity().GetNetworkState()->GetGameState()};
		entIterator.AttachFilter<EntityIteratorFilterComponent>(componentId);
		entIterator.AttachFilter<EntityIteratorFilterEntity>(m_kvNextNode);

		auto it = entIterator.begin();
		if(it != entIterator.end()) {
			auto *ent = *it;
			auto *ptrComponentPathNode = ent->FindComponent(componentId).get();
			if(ptrComponentPathNode != nullptr) {
				m_nextNode = std::static_pointer_cast<BasePointPathNodeComponent>(ptrComponentPathNode->shared_from_this());
				m_nextNode->SetPreviousNode(this);
			}
		}
	}
}

void BasePointPathNodeComponent::SetPreviousNode(BasePointPathNodeComponent *node) { m_previousNode = std::static_pointer_cast<BasePointPathNodeComponent>(node->shared_from_this()); }

BasePointPathNodeComponent *BasePointPathNodeComponent::GetPreviousNode() { return m_previousNode.get(); }
BasePointPathNodeComponent *BasePointPathNodeComponent::GetNextNode() { return m_nextNode.get(); }
