/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/audio/env_soundscape.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

void BaseEnvSoundScapeComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "soundscape", false))
			m_kvSoundScape = kvData.value;
		else if(ustring::compare<std::string>(kvData.key, "radius", false))
			m_kvRadius = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "position", false)) {
			auto id = util::to_int(kvData.key.substr(8, kvData.key.length()).c_str());
			m_positions.insert(std::unordered_map<unsigned int, std::string>::value_type(id, kvData.value));
		}
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
}
