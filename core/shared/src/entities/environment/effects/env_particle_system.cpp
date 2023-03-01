/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/effects/env_particle_system.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/asset/util_asset.hpp"
#include "pragma/networking/nwm_util.h"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include <algorithm>

using namespace pragma;

void BaseEnvParticleSystemComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseEnvParticleSystemComponent;

	{
		using TParticleSystem = std::string;
		auto memberInfo
		  = create_component_member_info<T, TParticleSystem, [](const ComponentMemberInfo &, T &c, TParticleSystem systemName) { c.SetParticleSystem(systemName); }, [](const ComponentMemberInfo &, T &c, TParticleSystem &value) { value = c.GetParticleSystem(); }>("particleSystem", "");
		registerMember(std::move(memberInfo));
	}
	{
		using TParticleSystemFile = std::string;
		auto memberInfo = create_component_member_info<T, TParticleSystemFile, [](const ComponentMemberInfo &, T &c, TParticleSystemFile fileName) { c.SetParticleFile(fileName); }, [](const ComponentMemberInfo &, T &c, TParticleSystemFile &value) { value = c.GetParticleFile(); }>(
		  "particleSystemFile", "", AttributeSpecializationType::File);
		auto &metaData = memberInfo.AddMetaData();
		metaData["assetType"] = "particlesystem";
		metaData["rootPath"] = util::Path::CreatePath(pragma::asset::get_asset_root_directory(pragma::asset::Type::ParticleSystem)).GetString();
		metaData["extensions"] = pragma::asset::get_supported_extensions(pragma::asset::Type::ParticleSystem, pragma::asset::FormatType::All);
		metaData["stripRootPath"] = true;
		metaData["stripExtension"] = true;
		registerMember(std::move(memberInfo));
	}
	{
		using TContinuous = bool;
		auto memberInfo = create_component_member_info<T, TContinuous, [](const ComponentMemberInfo &, T &c, TContinuous enabled) { c.SetContinuous(enabled); }, [](const ComponentMemberInfo &, T &c, TContinuous &value) { value = c.IsContinuous(); }>("continuous", "");
		registerMember(std::move(memberInfo));
	}
}
void BaseEnvParticleSystemComponent::SetParticleSystem(const std::string &ptName) { m_particleName = ptName; }
const std::string &BaseEnvParticleSystemComponent::GetParticleSystem() const { return m_particleName; }
void BaseEnvParticleSystemComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "particle", false))
			m_particleName = kvData.value;
		else if(ustring::compare<std::string>(kvData.key, "particle_file", false))
			SetParticleFile(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(ustring::compare<std::string>(inputData.input, "setcontinuous", false)) {
			auto b = (util::to_int(inputData.data) == 0) ? false : true;
			SetContinuous(b);
		}
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("io");
	ent.AddComponent("toggle");
	ent.AddComponent("transform");
}

void BaseEnvParticleSystemComponent::SetParticleFile(const std::string &fileName) { m_particleFile = fileName; }
const std::string &BaseEnvParticleSystemComponent::GetParticleFile() const { return m_particleFile; }

void BaseEnvParticleSystemComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto flags = GetEntity().GetSpawnFlags();
	UpdateRemoveOnComplete();
	auto *pToggleComponent = static_cast<pragma::BaseToggleComponent *>(GetEntity().FindComponent("toggle").get());
	if((pToggleComponent == nullptr || pToggleComponent->IsTurnedOn()) && GetRemoveOnComplete())
		GetEntity().RemoveSafely();
}

void BaseEnvParticleSystemComponent::UpdateRemoveOnComplete()
{
	auto flags = GetEntity().GetSpawnFlags();
	if(!(flags & SF_PARTICLE_SYSTEM_CONTINUOUS) && (flags & SF_PARTICLE_SYSTEM_REMOVE_ON_COMPLETE))
		SetRemoveOnComplete(true);
}

bool BaseEnvParticleSystemComponent::IsContinuous() const
{
	auto &ent = GetEntity();
	auto spawnFlags = ent.GetSpawnFlags();
	return (spawnFlags & SF_PARTICLE_SYSTEM_CONTINUOUS) != 0u;
}

void BaseEnvParticleSystemComponent::SetContinuous(bool b)
{
	auto &ent = GetEntity();
	auto spawnFlags = ent.GetSpawnFlags();
	if(b == true)
		spawnFlags |= SF_PARTICLE_SYSTEM_CONTINUOUS;
	else
		spawnFlags &= ~SF_PARTICLE_SYSTEM_CONTINUOUS;
	ent.SetSpawnFlags(spawnFlags);

	UpdateRemoveOnComplete();
	auto *pToggleComponent = static_cast<pragma::BaseToggleComponent *>(GetEntity().FindComponent("toggle").get());
	if((pToggleComponent == nullptr || pToggleComponent->IsTurnedOn()) && GetRemoveOnComplete())
		ent.RemoveSafely();
}

void BaseEnvParticleSystemComponent::SetRemoveOnComplete(bool b) { m_bRemoveOnComplete = b; }
bool BaseEnvParticleSystemComponent::GetRemoveOnComplete() const { return m_bRemoveOnComplete; }
