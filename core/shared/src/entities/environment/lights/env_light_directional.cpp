#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <sharedutils/util.h>
#include <algorithm>

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

void BaseEnvLightDirectionalComponent::SetAmbientColor(const Color &color) {*m_ambientColor = color;}
const Color &BaseEnvLightDirectionalComponent::GetAmbientColor() const {return *m_ambientColor;}
const util::PColorProperty &BaseEnvLightDirectionalComponent::GetAmbientColorProperty() const {return m_ambientColor;}
