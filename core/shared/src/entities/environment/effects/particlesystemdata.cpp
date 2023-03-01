/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/entities/environment/effects/particlesystemdata.h"
#include "pragma/asset/util_asset.hpp"
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util_file.h>
#include <sharedutils/util_path.hpp>
#include <udm.hpp>

// See c_particlesystem_save.cpp as well
bool pragma::asset::save_particle_system(VFilePtrReal &f, const std::unordered_map<std::string, CParticleSystemData> &particles, const std::string &rootPath)
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
bool pragma::asset::save_particle_system(const std::string &name, const std::unordered_map<std::string, CParticleSystemData> &particles, const std::string &rootPath)
{
	auto fpath = util::Path::CreatePath(rootPath) + (pragma::asset::get_normalized_path(name, pragma::asset::Type::ParticleSystem) + '.' + std::string {pragma::asset::FORMAT_PARTICLE_SYSTEM_ASCII});
	auto f = FileManager::OpenFile<VFilePtrReal>(fpath.GetString().c_str(), "w");
	if(!f)
		return false;
	return save_particle_system(f, particles, rootPath);
}
bool pragma::asset::save_particle_system(const CParticleSystemData &data, udm::AssetDataArg outData, std::string &outErr)
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
