// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.effects.particle_system;

export import pragma.udm;

export namespace pragma::asset {
	struct DLLNETWORK ParticleModifierData {
		ParticleModifierData(std::string pName) : name(pName) {}
		std::string name;
		std::unordered_map<std::string, std::string> settings;
	};

	struct DLLNETWORK ParticleChildData {
		std::string childName {};
		float delay = 0.f;
	};

	struct DLLNETWORK ParticleSystemData {
		std::unordered_map<std::string, std::string> settings;
		std::vector<ParticleModifierData> initializers;
		std::vector<ParticleModifierData> operators;
		std::vector<ParticleModifierData> renderers;
		std::vector<ParticleChildData> children;
	};

	CONSTEXPR_DLL_COMPAT uint32_t PPTSYS_FORMAT_VERSION = 1u;
	CONSTEXPR_DLL_COMPAT auto PPTSYS_COLLECTION_IDENTIFIER = "PPTSYSCOLLECTION";
	CONSTEXPR_DLL_COMPAT auto PPTSYS_IDENTIFIER = "PPTSYS";
	DLLNETWORK bool save_particle_system(const ParticleSystemData &data, udm::AssetDataArg outData, std::string &outErr);
	DLLNETWORK bool save_particle_system(const std::string &name, const std::unordered_map<std::string, ParticleSystemData> &particles, const std::string &rootPath = "");
	DLLNETWORK bool save_particle_system(fs::VFilePtrReal &f, const std::unordered_map<std::string, ParticleSystemData> &particles, const std::string &rootPath = "");
};
