#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light_spot_vol.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <algorithm>

using namespace pragma;

void BaseEnvLightSpotVolComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"cone_height",false))
			m_coneLength = ustring::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"cone_angle",false))
			m_coneAngle = ustring::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"cone_color",false))
			m_coneColor = Color(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
}

void BaseEnvLightSpotVolComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *pRenderComponent = dynamic_cast<pragma::BaseRenderComponent*>(&component);
	if(pRenderComponent != nullptr)
		pRenderComponent->SetCastShadows(false);
}
