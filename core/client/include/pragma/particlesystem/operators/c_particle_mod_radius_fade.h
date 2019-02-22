#ifndef __C_PARTICLE_MOD_RADIUS_FADE_H__
#define __C_PARTICLE_MOD_RADIUS_FADE_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_gradual_fade.hpp"

class DLLCLIENT CParticleOperatorRadiusFadeBase
	: public CParticleOperator,
	public CParticleModifierComponentGradualFade
{
public:
	virtual void Simulate(CParticle &particle,double) override;
	virtual void Initialize(CParticle &particle) override;
protected:
	CParticleOperatorRadiusFadeBase(const std::string &identifier,pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual void ApplyRadius(CParticle &particle,float radius) const=0;
private:
	CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>,float> m_fRadiusStart;
	CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>,float> m_fRadiusEnd;
	std::unique_ptr<std::vector<float>> m_particleStartRadiuses = nullptr;
};

////////////////////////////

class DLLCLIENT CParticleOperatorRadiusFade
	: public CParticleOperatorRadiusFadeBase
{
public:
	CParticleOperatorRadiusFade(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
protected:
	virtual void ApplyRadius(CParticle &particle,float radius) const override;
};

////////////////////////////

class DLLCLIENT CParticleOperatorLengthFade
	: public CParticleOperatorRadiusFadeBase
{
public:
	CParticleOperatorLengthFade(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
protected:
	virtual void ApplyRadius(CParticle &particle,float radius) const override;
};

#endif
