/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/environment/effects/particlesystemdata.h"
#include "pragma/asset/util_asset.hpp"
#include <sharedutils/util_file.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <udm.hpp>

using namespace pragma;

// See c_lengine.cpp as well
bool CParticleSystemComponent::Save(VFilePtrReal &f,const std::vector<CParticleSystemComponent*> &particleSystems)
{
	for(auto *ps : particleSystems)
	{
		if(ps->IsRecordingKeyValues() == false)
			return false;
	}

	std::unordered_map<std::string,CParticleSystemData> particles;
	for(auto *ps : particleSystems)
	{
		CParticleSystemData data {};
		ps->ToParticleSystemData(data);
		particles[ps->GetParticleSystemName()] = std::move(data);
	}
	return pragma::asset::save_particle_system(f,particles);
}
void CParticleSystemComponent::ToParticleSystemData(CParticleSystemData &outData)
{
	auto fToDataModifier = [](auto &modifiers,auto &outModifiers) {
		outModifiers.reserve(modifiers.size());
		for(auto &modifier : modifiers)
		{
			outModifiers.push_back({modifier->GetName()});
			auto &dtInitializer = outModifiers.back();
			dtInitializer.settings = *modifier->GetKeyValues();
		}
	};
	outData.settings = *GetKeyValues();
	fToDataModifier(GetInitializers(),outData.initializers);
	fToDataModifier(GetOperators(),outData.operators);
	fToDataModifier(GetRenderers(),outData.renderers);

	auto &children = GetChildren();
	outData.children.reserve(children.size());
	for(auto &child : children)
	{
		if(child.child.expired())
			continue;
		outData.children.push_back({});
		auto &dtChild = outData.children.back();
		dtChild.childName = child.child->GetParticleSystemName();
		dtChild.delay = child.delay;
	}
}
bool CParticleSystemComponent::Save(const std::string &fileName,const std::vector<CParticleSystemComponent*> &particleSystems)
{
	for(auto *ps : particleSystems)
	{
		if(ps->IsRecordingKeyValues() == false)
			return false;
	}
	auto ptPath = pragma::asset::get_normalized_path("particles/" +fileName,pragma::asset::Type::ParticleSystem) +'.' +pragma::asset::FORMAT_PARTICLE_SYSTEM_ASCII;
	auto f = FileManager::OpenFile<VFilePtrReal>(ptPath.c_str(),"w");
	if(!f)
		return false;
	return Save(f,particleSystems);
}
