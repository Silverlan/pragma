#ifndef __C_PARTICLESYSTEMDATA_H__
#define __C_PARTICLESYSTEMDATA_H__

#include "pragma/clientdefinitions.h"
#include <unordered_map>
#include <string>
#include <vector>

struct DLLCLIENT CParticleModifierData
{
	CParticleModifierData(std::string pName)
		: name(pName)
	{}
	std::string name;
	std::unordered_map<std::string,std::string> settings;
};

struct DLLCLIENT CParticleSystemData
{
	CParticleSystemData()=default;
	CParticleSystemData(CParticleSystemData&)=delete;
	CParticleSystemData &operator=(const CParticleSystemData&)=delete;
	std::unordered_map<std::string,std::string> settings;
	std::vector<std::unique_ptr<CParticleModifierData>> initializers;
	std::vector<std::unique_ptr<CParticleModifierData>> operators;
	std::vector<std::unique_ptr<CParticleModifierData>> renderers;
	std::vector<std::string> children;
};

#endif