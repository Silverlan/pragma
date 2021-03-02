/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/env_wind.hpp"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"

using namespace pragma;

void BaseEnvWindComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"wind_direction",false))
			SetWindDirection(uvec::create(kvData.value));
		else if(ustring::compare(kvData.key,"wind_speed",false))
			SetWindSpeed(util::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void BaseEnvWindComponent::SetWindForce(const Vector3 &force) {m_windForce = force;}
void BaseEnvWindComponent::SetWindDirection(const Vector3 &dir) {SetWindForce(dir *GetWindSpeed());}
void BaseEnvWindComponent::SetWindSpeed(float speed) {SetWindForce(GetWindDirection() *speed);}
const Vector3 &BaseEnvWindComponent::GetWindForce() const {return m_windForce;}
Vector3 BaseEnvWindComponent::GetWindDirection() const {return uvec::get_normal(GetWindForce());}
float BaseEnvWindComponent::GetWindSpeed() const {return uvec::length(GetWindForce());}
