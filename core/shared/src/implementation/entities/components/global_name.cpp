// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.global_name;

using namespace pragma;

GlobalNameComponent::GlobalNameComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void GlobalNameComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "globalname", false))
			m_globalName = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

const std::string &GlobalNameComponent::GetGlobalName() const { return m_globalName; }
void GlobalNameComponent::SetGlobalName(const std::string &name) { m_globalName = name; }
void GlobalNameComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

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
