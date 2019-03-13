#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light_spot_vol.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include <algorithm>

using namespace pragma;

#pragma optimize("",off)
void BaseEnvLightSpotVolComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"cone_height",false))
			GetEntity().SetKeyValue("radius",kvData.value);
		else if(ustring::compare(kvData.key,"cone_angle",false))
			m_coneAngle = ustring::to_float(kvData.value);
		else if(ustring::compare(kvData.key,"cone_color",false))
			GetEntity().SetKeyValue("color",kvData.value);
		else if(ustring::compare(kvData.key,"spotlight_target",false))
			m_kvSpotlightTargetName = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("toggle");
	ent.AddComponent("transform");
	ent.AddComponent("color");
	ent.AddComponent("point_at_target");
	auto *pRadiusComponent = dynamic_cast<pragma::BaseRadiusComponent*>(ent.AddComponent("radius").get());
	if(pRadiusComponent != nullptr)
		pRadiusComponent->SetRadius(100.f);
	m_netEvSetSpotlightTarget = SetupNetEvent("set_spotlight_target");
}

BaseEntity *BaseEnvLightSpotVolComponent::GetSpotlightTarget() const {return m_hSpotlightTarget.get();}

void BaseEnvLightSpotVolComponent::SetSpotlightTarget(BaseEntity &ent)
{
	m_hSpotlightTarget = ent.GetHandle();
}

void BaseEnvLightSpotVolComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(m_kvSpotlightTargetName.empty() == false)
	{
		EntityIterator entIt {*GetEntity().GetNetworkState()->GetGameState(),EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterEntity>(m_kvSpotlightTargetName);
		auto it = entIt.begin();
		if(it != entIt.end())
			SetSpotlightTarget(**it);
	}
}

void BaseEnvLightSpotVolComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *pRenderComponent = dynamic_cast<pragma::BaseRenderComponent*>(&component);
	if(pRenderComponent != nullptr)
		pRenderComponent->SetCastShadows(false);
}
#pragma optimize("",on)
