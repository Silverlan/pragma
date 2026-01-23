// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_name;

using namespace pragma;

void BaseNameComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { baseNameComponent::EVENT_ON_NAME_CHANGED = registerEvent("ON_NAME_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BaseNameComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember) { using T = BaseNameComponent; }
BaseNameComponent::BaseNameComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_name(util::StringProperty::Create()) {}
BaseNameComponent::~BaseNameComponent()
{
	if(m_cbOnNameChanged.IsValid())
		m_cbOnNameChanged.Remove();
}
void BaseNameComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "name", false) || pragma::string::compare<std::string>(kvData.key, "targetname", false))
			*m_name = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(baseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "setname", false))
			*m_name = inputData.data;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	m_cbOnNameChanged = m_name->AddCallback([this](std::reference_wrapper<const std::string> oldName, std::reference_wrapper<const std::string> newName) {
		CEOnNameChanged onNameChanged {newName.get()};
		BroadcastEvent(baseNameComponent::EVENT_ON_NAME_CHANGED, onNameChanged);
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
void CEOnNameChanged::PushArguments(lua::State *l) { Lua::PushString(l, name); }
