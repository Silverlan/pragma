/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/global_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

using namespace pragma;

GlobalNameComponent::GlobalNameComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void GlobalNameComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "globalname", false))
			m_globalName = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

const std::string &GlobalNameComponent::GetGlobalName() const { return m_globalName; }
void GlobalNameComponent::SetGlobalName(const std::string &name) { m_globalName = name; }
void GlobalNameComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void GlobalNameComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["name"] = GetGlobalName();
}

void GlobalNameComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	auto name = m_globalName;
	udm["name"](name);
	SetGlobalName(name);
}
