// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/environment/env_timescale.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

void BaseEnvTimescaleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "timescale", false))
			m_kvTimescale = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "inner_radius", false))
			m_kvInnerRadius = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "outer_radius", false))
			m_kvOuterRadius = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
