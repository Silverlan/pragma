// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/entities/basefilterentity.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/components/base_name_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <algorithm>

module pragma.entities.components.filter;

using namespace pragma;

void BaseFilterComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "entfilter", false)) {
			m_kvFilter = kvData.value;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
}

bool BaseFilterNameComponent::ShouldPass(BaseEntity &ent) const
{
	auto pNameComponent = static_cast<pragma::BaseNameComponent *>(ent.FindComponent("name").get());
	return pNameComponent != nullptr && ustring::match(pNameComponent->GetName(), m_kvFilter);
}

bool BaseFilterClassComponent::ShouldPass(BaseEntity &ent) const { return ustring::match(ent.GetClass().c_str(), m_kvFilter.c_str()); }
