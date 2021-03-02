/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_operator_toroidal_vortex.hpp"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

REGISTER_PARTICLE_OPERATOR(toroidal_vortex,CParticleOperatorToroidalVortex);

void CParticleOperatorToroidalVortex::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleOperatorWorldBase::Initialize(pSystem,values);
	for(auto it=values.begin();it!=values.end();it++)
	{
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "axis")
			m_vAxis = uvec::get_normal(uvec::create(it->second));
		else if(key == "strength")
			m_fStrength = util::to_float(it->second);
		else if(key == "divergence")
			m_fDivergence = util::to_float(it->second);
		else if(key == "height")
			m_fHeight = util::to_float(it->second);
		else if(key == "radius")
			m_fRadius = util::to_float(it->second);
	}
}
void CParticleOperatorToroidalVortex::Simulate(double tDelta)
{
	CParticleOperatorWorldBase::Simulate(tDelta);
	m_dtStrength = m_fStrength *tDelta;

	// transform the origin and axis into particle space
	auto &ps = GetParticleSystem();
	m_dtOrigin = ps.PointToParticleSpace(Vector3{},true);
	m_dtAxis = ps.DirectionToParticleSpace(m_vAxis,ShouldRotateWithEmitter());

	// find divergence rotation
	m_dtRotation = uquat::create(m_dtAxis,-m_fDivergence);
}
void CParticleOperatorToroidalVortex::Simulate(CParticle &particle,double tDelta,float strength)
{
	CParticleOperatorWorldBase::Simulate(particle,tDelta,strength);
	// cross product of ring axis and particle position is tangent
	auto pos = particle.GetPosition() -m_dtOrigin;
	auto tangent = uvec::cross(m_dtAxis,pos);
	auto l = uvec::length(tangent);
	const auto EPSILON = 0.0001f;
	if(l < EPSILON)
		return; // particle is on the axis
	tangent *= 1.f /l;

	// cross product of tangent and axis is direction from axis to position
	auto v = uvec::cross(tangent,m_dtAxis);

	// find vector from closest point on ring to position
	v *= m_fRadius;
	v += m_dtAxis *m_fHeight;
	v -= pos;
	l = uvec::length(v);
	if(l < EPSILON)
		return; // particle is on the ring
	v *= 1.f /l;

	// compute the rotation angle
	auto rot = uquat::create(tangent,m_fDivergence);

	// cross product of vector and tangent is direction
	v = uvec::cross(v,tangent) *static_cast<float>(tDelta);
	uvec::rotate(&v,rot);
	particle.SetVelocity(particle.GetVelocity() +v);
}
