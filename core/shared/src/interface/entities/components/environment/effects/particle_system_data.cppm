// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.environment.effects.particle_system;

export import pragma.udm;

export {
	struct DLLNETWORK CParticleModifierData {
		CParticleModifierData(std::string pName) : name(pName) {}
		std::string name;
		std::unordered_map<std::string, std::string> settings;
	};

	struct DLLNETWORK CParticleChildData {
		std::string childName {};
		float delay = 0.f;
	};

	struct DLLNETWORK CParticleSystemData {
		std::unordered_map<std::string, std::string> settings;
		std::vector<CParticleModifierData> initializers;
		std::vector<CParticleModifierData> operators;
		std::vector<CParticleModifierData> renderers;
		std::vector<CParticleChildData> children;
	};

	namespace pragma {
		namespace asset {
			constexpr uint32_t PPTSYS_FORMAT_VERSION = 1u;
			constexpr auto PPTSYS_COLLECTION_IDENTIFIER = "PPTSYSCOLLECTION";
			constexpr auto PPTSYS_IDENTIFIER = "PPTSYS";
			DLLNETWORK bool save_particle_system(const CParticleSystemData &data, udm::AssetDataArg outData, std::string &outErr);
			DLLNETWORK bool save_particle_system(const std::string &name, const std::unordered_map<std::string, CParticleSystemData> &particles, const std::string &rootPath = "");
			DLLNETWORK bool save_particle_system(VFilePtrReal &f, const std::unordered_map<std::string, CParticleSystemData> &particles, const std::string &rootPath = "");
		};
	};
};
