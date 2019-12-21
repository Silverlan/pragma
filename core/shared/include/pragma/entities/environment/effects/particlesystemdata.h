#ifndef __PARTICLESYSTEMDATA_H__
#define __PARTICLESYSTEMDATA_H__

#include "pragma/networkdefinitions.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

struct DLLNETWORK CParticleModifierData
{
	CParticleModifierData(std::string pName)
		: name(pName)
	{}
	std::string name;
	std::unordered_map<std::string,std::string> settings;
};

struct DLLNETWORK CParticleSystemData
{
	std::unordered_map<std::string,std::string> settings;
	std::vector<CParticleModifierData> initializers;
	std::vector<CParticleModifierData> operators;
	std::vector<CParticleModifierData> renderers;
	std::vector<std::string> children;
};

namespace pragma
{
	namespace asset
	{
		DLLNETWORK void get_particle_system_file_path(std::string &path);
		DLLNETWORK bool save_particle_system(const std::string &name,const std::unordered_map<std::string,CParticleSystemData> &particles);
	};
};

#endif