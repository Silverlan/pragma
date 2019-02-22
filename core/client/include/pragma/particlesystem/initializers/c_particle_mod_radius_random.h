#ifndef __C_PARTICLE_MOD_RADIUS_RANDOM_H__
#define __C_PARTICLE_MOD_RADIUS_RANDOM_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

class DLLCLIENT CParticleInitializerRadiusRandomBase
	: public CParticleInitializer
{
public:
	CParticleInitializerRadiusRandomBase(const std::string &identifier,pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual void Initialize(CParticle &particle) override;
	float GetMinRadius() const;
	float GetMaxRadius() const;
protected:
	virtual void ApplyRadius(CParticle &particle,float radius)=0;
private:
	float m_radiusMin;
	float m_radiusMax;
};

////////////////////////////

class DLLCLIENT CParticleInitializerRadiusRandom
	: public CParticleInitializerRadiusRandomBase
{
public:
	CParticleInitializerRadiusRandom(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
protected:
	virtual void ApplyRadius(CParticle &particle,float radius) override;
};

////////////////////////////

class DLLCLIENT CParticleInitializerLengthRandom
	: public CParticleInitializerRadiusRandomBase
{
public:
	CParticleInitializerLengthRandom(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
protected:
	virtual void ApplyRadius(CParticle &particle,float radius) override;
};

#endif