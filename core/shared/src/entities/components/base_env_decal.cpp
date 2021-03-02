/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/env_decal.h"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

void BaseEnvDecalComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"material",false))
			m_material = kvData.value;
		else if(ustring::compare(kvData.key,"size",false))
			m_size = ustring::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"start_disabled",false))
			m_startDisabled = util::to_boolean(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("transform");
}

float BaseEnvDecalComponent::GetSize() const {return m_size;}
const std::string &BaseEnvDecalComponent::GetMaterial() const {return m_material;}
bool BaseEnvDecalComponent::GetStartDisabled() const {return m_startDisabled;}
