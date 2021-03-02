/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/effects/env_smoke_trail.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <algorithm>

using namespace pragma;

void BaseEnvSmokeTrailComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"speed",false))
			m_speed = ustring::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"distance",false))
			m_distance = ustring::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"min_sprite_size",false))
			m_minSpriteSize = ustring::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"max_sprite_size",false))
			m_maxSpriteSize = ustring::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"material",false))
			m_material = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
}
