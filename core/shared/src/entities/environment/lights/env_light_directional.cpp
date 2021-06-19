/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <sharedutils/util.h>
#include <algorithm>
#include <udm.hpp>

using namespace pragma;

BaseEnvLightDirectionalComponent::BaseEnvLightDirectionalComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_ambientColor(util::ColorProperty::Create(Color(255,255,255,200)))
{}
void BaseEnvLightDirectionalComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"color_ambient",false))
			*m_ambientColor = Color(kvData.value);
		else if(ustring::compare(kvData.key,"max_exposure",false))
			m_maxExposure = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("light");
	m_netEvSetAmbientColor = SetupNetEvent("set_ambient_color");
}

void BaseEnvLightDirectionalComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["maxExposure"] = m_maxExposure;
	udm["ambientColor"] = (*m_ambientColor)->ToVector4();
}
void BaseEnvLightDirectionalComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);
	udm["maxExposure"](m_maxExposure);
	Vector4 color;
	udm["ambientColor"](color);
	*m_ambientColor = Color{color};
}

void BaseEnvLightDirectionalComponent::SetAmbientColor(const Color &color) {*m_ambientColor = color;}
const Color &BaseEnvLightDirectionalComponent::GetAmbientColor() const {return *m_ambientColor;}
const util::PColorProperty &BaseEnvLightDirectionalComponent::GetAmbientColorProperty() const {return m_ambientColor;}
