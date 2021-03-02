/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/env_fog_controller.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/components/basetoggle.h"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/components/base_io_component.hpp"

using namespace pragma;

void BaseEnvFogControllerComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"fogcolor",false))
			GetEntity().SetKeyValue("color",kvData.value);
		else if(ustring::compare(kvData.key,"fogstart",false))
			m_kvFogStart = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"fogend",false))
			m_kvFogEnd = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"fogmaxdensity",false))
			m_kvMaxDensity = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"fogtype",false))
			m_kvFogType = CUChar(util::to_int(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData&>(evData.get());
		if(ustring::compare(inputData.input,"setstartdist",false))
			m_kvFogStart = util::to_float(inputData.data);
		else if(ustring::compare(inputData.input,"setenddist",false))
			m_kvFogEnd = util::to_float(inputData.data);
		else if(ustring::compare(inputData.input,"setmaxdensity",false))
			m_kvMaxDensity = util::to_float(inputData.data);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("io");
	ent.AddComponent("toggle");
	ent.AddComponent("color");
}

void BaseEnvFogControllerComponent::SetFogStart(float start)
{
	m_kvFogStart = start;
}
void BaseEnvFogControllerComponent::SetFogEnd(float end)
{
	m_kvFogEnd = end;
}
void BaseEnvFogControllerComponent::SetMaxDensity(float density)
{
	m_kvMaxDensity = density;
}
void BaseEnvFogControllerComponent::SetFogType(unsigned char type)
{
	m_kvFogType = type;
}