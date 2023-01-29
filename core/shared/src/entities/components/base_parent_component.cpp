/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_parent_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_attachable_component.hpp"

using namespace pragma;

BaseParentComponent::BaseParentComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void BaseParentComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateChildAttachmentData(); });
}
void BaseParentComponent::UpdateChildAttachmentData()
{
	auto *pAttachableComponent = static_cast<BaseAttachableComponent *>(GetEntity().FindComponent("attachable").get());
	if(pAttachableComponent != nullptr)
		pAttachableComponent->UpdateAttachmentData(true);
	for(auto &hChild : m_children) {
		if(hChild.expired())
			continue;
		hChild->UpdateAttachmentData(true);
	}
}
const std::vector<util::TWeakSharedHandle<BaseAttachableComponent>> &BaseParentComponent::GetChildren() const { return const_cast<BaseParentComponent *>(this)->GetChildren(); }
std::vector<util::TWeakSharedHandle<BaseAttachableComponent>> &BaseParentComponent::GetChildren() { return m_children; }
void BaseParentComponent::RemoveChild(BaseAttachableComponent &child)
{
	auto it = std::find_if(m_children.begin(), m_children.end(), [&child](const ComponentHandle<BaseAttachableComponent> &component) { return &child == component.get(); });
	if(it != m_children.end())
		m_children.erase(it);
}
void BaseParentComponent::AddChild(BaseAttachableComponent &child) { m_children.push_back(child.GetHandle<BaseAttachableComponent>()); }
