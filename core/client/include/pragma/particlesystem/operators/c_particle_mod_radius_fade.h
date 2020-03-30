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
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void Simulate(CParticle &particle,double) override;
	virtual void OnParticleCreated(CParticle &particle) override;
protected:
	CParticleOperatorRadiusFadeBase(const std::string &identifier);
	virtual void ApplyRadius(CParticle &particle,float radius) const=0;
private:
	CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>,float> m_fRadiusStart;
	CParticleModifierComponentRandomVariable<std::uniform_real_distribution<float>,float> m_fRadiusEnd;
	std::unique_ptr<std::vector<float>> m_particleStartRadiuses = nullptr;
	std::string m_identifier;
};

////////////////////////////

class DLLCLIENT CParticleOperatorRadiusFade
	: public CParticleOperatorRadiusFadeBase
{
public:
	CParticleOperatorRadiusFade();
protected:
	virtual void ApplyRadius(CParticle &particle,float radius) const override;
};

////////////////////////////

class DLLCLIENT CParticleOperatorLengthFade
	: public CParticleOperatorRadiusFadeBase
{
public:
	CParticleOperatorLengthFade();
protected:
	virtual void ApplyRadius(CParticle &particle,float radius) const override;
};

#endif
