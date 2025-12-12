// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_filter;

using namespace pragma;

void BaseFilterComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "entfilter", false)) {
			m_kvFilter = kvData.value;
			return pragma::util::EventReply::Handled;
		}
		return pragma::util::EventReply::Unhandled;
	});
}

bool BaseFilterNameComponent::ShouldPass(pragma::ecs::BaseEntity &ent) const
{
	auto pNameComponent = static_cast<pragma::BaseNameComponent *>(ent.FindComponent("name").get());
	return pNameComponent != nullptr && pragma::string::match(pNameComponent->GetName(), m_kvFilter);
}

bool BaseFilterClassComponent::ShouldPass(pragma::ecs::BaseEntity &ent) const { return pragma::string::match(ent.GetClass().c_str(), m_kvFilter.c_str()); }
