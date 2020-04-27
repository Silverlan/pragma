/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/render_component_flags.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <sharedutils/datastream.h>

using namespace pragma;

void BaseRenderComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"disableshadows",false))
			SetCastShadows(!util::to_boolean(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
void BaseRenderComponent::SetUnlit(bool b)
{
	if(b)
		m_renderFlags |= FRenderFlags::Unlit;
	else
		m_renderFlags &= ~FRenderFlags::Unlit;
}
void BaseRenderComponent::SetCastShadows(bool b)
{
	if(b)
		m_renderFlags |= FRenderFlags::CastShadows;
	else
		m_renderFlags &= ~FRenderFlags::CastShadows;
}
bool BaseRenderComponent::IsUnlit() const {return (m_renderFlags &FRenderFlags::Unlit) != FRenderFlags::None;}
bool BaseRenderComponent::GetCastShadows() const {return (m_renderFlags &FRenderFlags::CastShadows) != FRenderFlags::None;}

void BaseRenderComponent::Save(DataStream &ds)
{
	BaseEntityComponent::Save(ds);
	ds->Write<decltype(m_renderFlags)>(m_renderFlags);
}
void BaseRenderComponent::Load(DataStream &ds,uint32_t version)
{
	BaseEntityComponent::Load(ds,version);
	m_renderFlags = ds->Read<decltype(m_renderFlags)>();
}
