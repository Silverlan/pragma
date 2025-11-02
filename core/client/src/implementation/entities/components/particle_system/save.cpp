// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.client;


import :entities.components.particle_system;
using namespace pragma;

// See c_lengine.cpp as well
bool ecs::CParticleSystemComponent::Save(VFilePtrReal &f, const std::vector<ecs::CParticleSystemComponent *> &particleSystems)
{
	for(auto *ps : particleSystems) {
		if(ps->IsRecordingKeyValues() == false)
			return false;
	}

	std::unordered_map<std::string, CParticleSystemData> particles;
	for(auto *ps : particleSystems) {
		CParticleSystemData data {};
		ps->ToParticleSystemData(data);
		particles[ps->GetParticleSystemName()] = std::move(data);
	}
	return pragma::asset::save_particle_system(f, particles);
}
void ecs::CParticleSystemComponent::ToParticleSystemData(CParticleSystemData &outData)
{
	auto fToDataModifier = [](auto &modifiers, auto &outModifiers) {
		outModifiers.reserve(modifiers.size());
		for(auto &modifier : modifiers) {
			outModifiers.push_back({modifier->GetName()});
			auto &dtInitializer = outModifiers.back();
			dtInitializer.settings = *modifier->GetKeyValues();
		}
	};
	outData.settings = *GetKeyValues();
	fToDataModifier(GetInitializers(), outData.initializers);
	fToDataModifier(GetOperators(), outData.operators);
	fToDataModifier(GetRenderers(), outData.renderers);

	auto &children = GetChildren();
	outData.children.reserve(children.size());
	for(auto &child : children) {
		if(child.child.expired())
			continue;
		outData.children.push_back({});
		auto &dtChild = outData.children.back();
		dtChild.childName = child.child->GetParticleSystemName();
		dtChild.delay = child.delay;
	}
}
bool ecs::CParticleSystemComponent::Save(const std::string &fileName, const std::vector<ecs::CParticleSystemComponent *> &particleSystems)
{
	for(auto *ps : particleSystems) {
		if(ps->IsRecordingKeyValues() == false)
			return false;
	}
	auto ptPath = pragma::asset::get_normalized_path("particles/" + fileName, pragma::asset::Type::ParticleSystem) + '.' + pragma::asset::FORMAT_PARTICLE_SYSTEM_ASCII;
	auto f = FileManager::OpenFile<VFilePtrReal>(ptPath.c_str(), "w");
	if(!f)
		return false;
	return Save(f, particleSystems);
}
