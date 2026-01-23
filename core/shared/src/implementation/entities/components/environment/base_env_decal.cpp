// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.base_decal;

using namespace pragma;

void BaseEnvDecalComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseEnvDecalComponent;
	{
		using TMaterial = std::string;
		auto memberInfo = create_component_member_info<T, TMaterial, static_cast<void (T::*)(const TMaterial &)>(&T::SetMaterial), static_cast<const TMaterial &(T::*)() const>(&T::GetMaterial)>("material", "", AttributeSpecializationType::File);
		auto &metaData = memberInfo.AddMetaData();
		metaData["assetType"] = "material";
		metaData["rootPath"] = util::Path::CreatePath(pragma::asset::get_asset_root_directory(asset::Type::Material)).GetString();
		metaData["extensions"] = pragma::asset::get_supported_extensions(asset::Type::Material, asset::FormatType::All);
		metaData["stripRootPath"] = true;
		metaData["stripExtension"] = true;
		registerMember(std::move(memberInfo));
	}

	{
		using TSize = float;
		auto memberInfo = create_component_member_info<T, TSize, static_cast<void (T::*)(TSize)>(&T::SetSize), static_cast<TSize (T::*)() const>(&T::GetSize)>("size", 1.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(50.f);
		registerMember(std::move(memberInfo));
	}
}
void BaseEnvDecalComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "material", false))
			m_material = kvData.value;
		else if(pragma::string::compare<std::string>(kvData.key, "size", false))
			m_size = string::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "start_disabled", false))
			m_startDisabled = util::to_boolean(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("transform");
}

void BaseEnvDecalComponent::SetSize(float size) { m_size = size; }
float BaseEnvDecalComponent::GetSize() const { return m_size; }
void BaseEnvDecalComponent::SetMaterial(const std::string &mat) { m_material = mat; }
const std::string &BaseEnvDecalComponent::GetMaterial() const { return m_material; }
bool BaseEnvDecalComponent::GetStartDisabled() const { return m_startDisabled; }
