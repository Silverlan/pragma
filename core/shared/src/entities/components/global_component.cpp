/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/global_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include <sharedutils/datastream.h>

using namespace pragma;

GlobalNameComponent::GlobalNameComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
void GlobalNameComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"globalname",false))
			m_globalName = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

const std::string &GlobalNameComponent::GetGlobalName() const {return m_globalName;}
void GlobalNameComponent::SetGlobalName(const std::string &name) {m_globalName = name;}
luabind::object GlobalNameComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<GlobalNameComponentHandleWrapper>(l);}

void GlobalNameComponent::Save(DataStream &ds)
{
	BaseEntityComponent::Save(ds);
	ds->WriteString(GetGlobalName());
}

void GlobalNameComponent::Load(DataStream &ds,uint32_t version)
{
	BaseEntityComponent::Load(ds,version);
	SetGlobalName(ds->ReadString());
}
