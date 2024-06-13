/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PARTICLESYSTEMDATA_H__
#define __PARTICLESYSTEMDATA_H__

#include "pragma/networkdefinitions.h"
#include <fsys/vfileptr.h>
#include <udm_types.hpp>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

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
		static constexpr uint32_t PPTSYS_FORMAT_VERSION = 1u;
		static constexpr auto PPTSYS_COLLECTION_IDENTIFIER = "PPTSYSCOLLECTION";
		static constexpr auto PPTSYS_IDENTIFIER = "PPTSYS";
		DLLNETWORK bool save_particle_system(const CParticleSystemData &data, udm::AssetDataArg outData, std::string &outErr);
		DLLNETWORK bool save_particle_system(const std::string &name, const std::unordered_map<std::string, CParticleSystemData> &particles, const std::string &rootPath = "");
		DLLNETWORK bool save_particle_system(VFilePtrReal &f, const std::unordered_map<std::string, CParticleSystemData> &particles, const std::string &rootPath = "");
	};
};

#endif
