#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/initializers/c_particle_mod_initial_animation_frame.hpp"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

REGISTER_PARTICLE_INITIALIZER(initial_animation_frame,CParticleInitializerInitialAnimationFrame);

void CParticleInitializerInitialAnimationFrame::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleInitializer::Initialize(pSystem,values);
	for(auto it=values.begin();it!=values.end();it++)
	{
		std::string key = it->first;
		std::transform(key.begin(),key.end(),key.begin(),::tolower);
		if(key == "frame_min")
		{
			m_bUseFraction = false;
			m_minFrame = util::to_int(it->second);
		}
		else if(key == "frame_min_fraction")
		{
			m_bUseFraction = true;
			m_minFrame = util::to_float(it->second);
		}
		else if(key == "frame_max")
		{
			m_bUseFraction = false;
			m_maxFrame = util::to_int(it->second);
		}
		else if(key == "frame_max_fraction")
		{
			m_bUseFraction = true;
			m_maxFrame = util::to_float(it->second);
		}
	}
}
void CParticleInitializerInitialAnimationFrame::OnParticleCreated(CParticle &particle)
{
	CParticleInitializer::OnParticleCreated(particle);
	auto &pSystem = GetParticleSystem();
	auto *animData = pSystem.GetAnimationData();
	if(animData == nullptr)
		return;
	if(m_bUseFraction == true)
	{
		particle.SetFrameOffset(umath::random(m_minFrame,m_maxFrame));
		return;
	}

	auto frameId = umath::random(umath::round(m_minFrame),umath::round(m_maxFrame));
	auto frameBias = (1.f /static_cast<float>(animData->frames)) *0.5f; // Add a frame bias to make sure we use the right frame instead of the frame before it due to potential precision errors
	particle.SetFrameOffset((animData->frames > 0) ? (frameId /static_cast<float>(animData->frames) +frameBias) : 0.f);
}
