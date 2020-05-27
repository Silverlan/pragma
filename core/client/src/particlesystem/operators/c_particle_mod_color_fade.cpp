/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_mod_color_fade.h"

REGISTER_PARTICLE_OPERATOR(color_fade,CParticleOperatorColorFade);

void CParticleOperatorColorFade::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleOperator::Initialize(pSystem,values);
	CParticleModifierComponentGradualFade::Initialize(values);
	m_colorStart.Initialize("start",values);
	m_colorEnd.Initialize("end",values);

	m_colorEnd.Initialize("",values); // Allows "color" as alternative to "color_end"
									  // If no start color has been specified, the previous known color of the particle has to be used as start color.
									  // Since that color cannot be known beforehand, we need to store it.
	if(m_colorStart.IsSet() == false)
		m_particleStartColors = std::make_unique<std::vector<Color>>(pSystem.GetMaxParticleCount(),Color(std::numeric_limits<int16_t>::max(),0,0,0));
}
void CParticleOperatorColorFade::OnParticleCreated(CParticle &particle)
{
	if(m_particleStartColors == nullptr)
		return;
	m_particleStartColors->at(particle.GetIndex()) = Color(std::numeric_limits<int16_t>::max(),0,0,0);
}
void CParticleOperatorColorFade::Simulate(CParticle &particle,double,float strength)
{
	auto tFade = 0.f;
	if(GetEasedFadeFraction(particle,tFade) == false)
		return;
	auto colorStart = Color{0,0,0,0};
	auto componentFlags = CParticleModifierComponentRandomColor::ComponentFlags::None;
	if(m_particleStartColors != nullptr)
	{
		// Use last known particle color
		auto &ptColorStart = m_particleStartColors->at(particle.GetIndex());
		if(ptColorStart.r == std::numeric_limits<int16_t>::max())
			ptColorStart = particle.GetColor();
		colorStart = ptColorStart;
		componentFlags |= CParticleModifierComponentRandomColor::ComponentFlags::RGBA;
	}
	else
	{
		colorStart = m_colorStart.GetValue(particle);
		componentFlags |= m_colorStart.GetComponentFlags();
	}
	componentFlags &= m_colorEnd.GetComponentFlags();
	auto newColor = colorStart.Lerp(m_colorEnd.GetValue(particle),tFade);

	auto color = colorStart;
	if((componentFlags &CParticleModifierComponentRandomColor::ComponentFlags::Red) != CParticleModifierComponentRandomColor::ComponentFlags::None)
		color.r = newColor.r;
	if((componentFlags &CParticleModifierComponentRandomColor::ComponentFlags::Green) != CParticleModifierComponentRandomColor::ComponentFlags::None)
		color.g = newColor.g;
	if((componentFlags &CParticleModifierComponentRandomColor::ComponentFlags::Blue) != CParticleModifierComponentRandomColor::ComponentFlags::None)
		color.b = newColor.b;
	if((componentFlags &CParticleModifierComponentRandomColor::ComponentFlags::Alpha) != CParticleModifierComponentRandomColor::ComponentFlags::None)
		color.a = newColor.a;
	particle.SetColor(color);
}

