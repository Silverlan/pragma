#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light_spot.h"
#include <sharedutils/util.h>
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <algorithm>

using namespace pragma;

BaseEnvLightSpotComponent::BaseEnvLightSpotComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_angInnerCutoff(util::FloatProperty::Create(0.f)),
	m_angOuterCutoff(util::FloatProperty::Create(0.f)),m_coneStartOffset(util::FloatProperty::Create(0.f))
{}
void BaseEnvLightSpotComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"outercutoff",false))
			*m_angOuterCutoff = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"innercutoff",false))
			*m_angInnerCutoff = util::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"cone_start_offset",false))
			SetConeStartOffset(util::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("light");
	ent.AddComponent("radius");
	ent.AddComponent("point_at_target");
	m_netEvSetConeStartOffset = SetupNetEvent("set_cone_start_offset");
}

void BaseEnvLightSpotComponent::SetOuterCutoffAngle(umath::Degree ang) {*m_angOuterCutoff = ang;}
umath::Degree BaseEnvLightSpotComponent::GetOuterCutoffAngle() const {return *m_angOuterCutoff;}
void BaseEnvLightSpotComponent::SetInnerCutoffAngle(umath::Degree ang) {*m_angInnerCutoff = ang;}
umath::Degree BaseEnvLightSpotComponent::GetInnerCutoffAngle() const {return *m_angInnerCutoff;}

void BaseEnvLightSpotComponent::SetConeStartOffset(float offset) {*m_coneStartOffset = offset;}
float BaseEnvLightSpotComponent::GetConeStartOffset() const {return *m_coneStartOffset;}
