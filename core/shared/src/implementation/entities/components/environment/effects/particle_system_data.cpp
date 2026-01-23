// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.effects.particle_system;

// See c_particlesystem_save.cpp as well
bool pragma::asset::save_particle_system(fs::VFilePtrReal &f, const std::unordered_map<std::string, ParticleSystemData> &particles, const std::string &rootPath)
{
	auto udmData = udm::Data::Create(PPTSYS_COLLECTION_IDENTIFIER, PPTSYS_FORMAT_VERSION);
	auto &udm = *udmData;
	auto udmParticleSystemDefinitions = udm["particleSystemDefinitions"];
	for(auto &pair : particles) {
		auto &name = pair.first;
		auto udmParticleSystemDefinition = udmParticleSystemDefinitions[name];
		std::string err;
		if(save_particle_system(pair.second, udm::AssetData {udmParticleSystemDefinition}, err) == false)
			return false;
	}
	return udmData->SaveAscii(f);
}
bool pragma::asset::save_particle_system(const std::string &name, const std::unordered_map<std::string, ParticleSystemData> &particles, const std::string &rootPath)
{
	auto fpath = util::Path::CreatePath(rootPath) + (get_normalized_path(name, Type::ParticleSystem) + '.' + std::string {FORMAT_PARTICLE_SYSTEM_ASCII});
	auto f = fs::open_file<fs::VFilePtrReal>(fpath.GetString().c_str(), fs::FileMode::Write);
	if(!f)
		return false;
	return save_particle_system(f, particles, rootPath);
}
bool pragma::asset::save_particle_system(const ParticleSystemData &data, udm::AssetDataArg outData, std::string &outErr)
{
	outData.SetAssetType(PPTSYS_IDENTIFIER);
	outData.SetAssetVersion(PPTSYS_FORMAT_VERSION);

	auto udm = *outData;
	udm["keyValues"] = data.settings;

	auto addModifiers = [&udm](const std::string &name, const auto &modifiers) {
		auto udmModifiers = udm.AddArray(name, modifiers.size());
		for(auto i = decltype(modifiers.size()) {0u}; i < modifiers.size(); ++i) {
			auto &modifier = modifiers[i];
			auto udmInitializer = udmModifiers[i];
			udmInitializer["name"] = modifier.name;
			udmInitializer["keyValues"] = modifier.settings;
		}
	};
	addModifiers("initializers", data.initializers);
	addModifiers("operators", data.operators);
	addModifiers("renderers", data.renderers);

	auto &children = data.children;
	auto numChildren = children.size();
	auto udmChildren = udm.AddArray("children", numChildren);
	uint32_t idx = 0;
	for(auto &child : children) {
		auto udmChild = udmChildren[idx++];
		udmChild["type"] = child.childName;
		udmChild["delay"] = child.delay;
	}
	return true;
}
