/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/effects/env_fire.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <algorithm>

using namespace pragma;

void BaseEnvFireComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("toggle");

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "fire_type", false))
			m_fireType = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
