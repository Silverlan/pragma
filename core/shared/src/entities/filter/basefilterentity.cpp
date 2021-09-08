/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/basefilterentity.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/components/base_name_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <algorithm>

using namespace pragma;

void BaseFilterComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare<std::string>(kvData.key,"entfilter",false))
		{
			m_kvFilter = kvData.value;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
}

bool BaseFilterNameComponent::ShouldPass(BaseEntity &ent) const
{
	auto pNameComponent = static_cast<pragma::BaseNameComponent*>(ent.FindComponent("name").get());
	return pNameComponent != nullptr && ustring::match(pNameComponent->GetName(),m_kvFilter);
}

bool BaseFilterClassComponent::ShouldPass(BaseEntity &ent) const
{
	return ustring::match(ent.GetClass(),m_kvFilter);
}
