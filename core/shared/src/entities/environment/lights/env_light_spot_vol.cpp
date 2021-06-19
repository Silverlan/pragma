/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light_spot_vol.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/util/util_game.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include <algorithm>
#include <udm.hpp>

using namespace pragma;

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
		else if(ustring::compare(kvData.key,"cone_start_offset",false))
			m_coneStartOffset = util::to_float(kvData.value);
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

void BaseEnvLightSpotVolComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["coneAngle"] = m_coneAngle;
	udm["coneStartOffset"] = m_coneStartOffset;
	udm["spotlightTargetName"] = m_kvSpotlightTargetName;
	util::write_udm_entity(udm["spotlightTarget"],m_hSpotlightTarget);
}
void BaseEnvLightSpotVolComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);
	udm["coneAngle"](m_coneAngle);
	udm["coneStartOffset"](m_coneStartOffset);
	udm["spotlightTargetName"](m_kvSpotlightTargetName);
	m_hSpotlightTarget = util::read_udm_entity(*this,udm["spotlightTarget"]);
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
