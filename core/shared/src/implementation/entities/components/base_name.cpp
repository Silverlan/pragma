// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cinttypes>
#include "pragma/lua/core.hpp"

#include <string>

module pragma.shared;

import :entities.components.base_name;

using namespace pragma;

ComponentEventId BaseNameComponent::EVENT_ON_NAME_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseNameComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_NAME_CHANGED = registerEvent("ON_NAME_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BaseNameComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember) { using T = BaseNameComponent; }
BaseNameComponent::BaseNameComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_name(util::StringProperty::Create()) {}
BaseNameComponent::~BaseNameComponent()
{
	if(m_cbOnNameChanged.IsValid())
		m_cbOnNameChanged.Remove();
}
void BaseNameComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "name", false) || ustring::compare<std::string>(kvData.key, "targetname", false))
			*m_name = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(ustring::compare<std::string>(inputData.input, "setname", false))
			*m_name = inputData.data;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	m_cbOnNameChanged = m_name->AddCallback([this](std::reference_wrapper<const std::string> oldName, std::reference_wrapper<const std::string> newName) {
		pragma::CEOnNameChanged onNameChanged {newName.get()};
		BroadcastEvent(EVENT_ON_NAME_CHANGED, onNameChanged);
	});
}

const std::string &BaseNameComponent::GetName() const { return *m_name; }
void BaseNameComponent::SetName(std::string name) { *m_name = name; }
const util::PStringProperty &BaseNameComponent::GetNameProperty() const { return m_name; }

void BaseNameComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["name"] = **m_name;
}

void BaseNameComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	std::string name;
	udm["name"](name);
	SetName(name);
}

/////////////////

CEOnNameChanged::CEOnNameChanged(const std::string &newName) : name {newName} {}
void CEOnNameChanged::PushArguments(lua_State *l) { Lua::PushString(l, name); }
