// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.point.base_path_node;

using namespace pragma;

void BasePointPathNodeComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "next_node", false))
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
		ecs::EntityIterator entIterator {*GetEntity().GetNetworkState()->GetGameState()};
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
