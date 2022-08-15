/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/env_decal.h"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/attribute_specialization_type.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include <pragma/asset/util_asset.hpp>

using namespace pragma;

void BaseEnvDecalComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember)
{
	using T = BaseEnvDecalComponent;
	{
		using TMaterial = std::string;
		auto memberInfo = create_component_member_info<
			T,TMaterial,
			static_cast<void(T::*)(const TMaterial&)>(&T::SetMaterial),
			static_cast<const TMaterial&(T::*)() const>(&T::GetMaterial)
		>("material","",AttributeSpecializationType::File);
		auto &metaData = memberInfo.AddMetaData();
		metaData["assetType"] = "material";
		metaData["rootPath"] = util::Path::CreatePath(pragma::asset::get_asset_root_directory(pragma::asset::Type::Material)).GetString();
		metaData["extensions"] = pragma::asset::get_supported_extensions(pragma::asset::Type::Material,pragma::asset::FormatType::All);
		metaData["stripRootPath"] = true;
		metaData["stripExtension"] = true;
		registerMember(std::move(memberInfo));
	}

	{
		using TSize = float;
		auto memberInfo = create_component_member_info<
			T,TSize,
			static_cast<void(T::*)(TSize)>(&T::SetSize),
			static_cast<TSize(T::*)() const>(&T::GetSize)
		>("size",1.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(50.f);
		registerMember(std::move(memberInfo));
	}
}
void BaseEnvDecalComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare<std::string>(kvData.key,"material",false))
			m_material = kvData.value;
		else if(ustring::compare<std::string>(kvData.key,"size",false))
			m_size = ustring::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key,"start_disabled",false))
			m_startDisabled = util::to_boolean(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("transform");
}

void BaseEnvDecalComponent::SetSize(float size) {m_size = size;}
float BaseEnvDecalComponent::GetSize() const {return m_size;}
void BaseEnvDecalComponent::SetMaterial(const std::string &mat) {m_material = mat;}
const std::string &BaseEnvDecalComponent::GetMaterial() const {return m_material;}
bool BaseEnvDecalComponent::GetStartDisabled() const {return m_startDisabled;}
