/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_mod_trail.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

REGISTER_PARTICLE_OPERATOR(trail, CParticleOperatorTrail);

void CParticleOperatorTrail::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto &it : values) {
		auto key = it.first;
		ustring::to_lower(key);
		if(key == "travel_time")
			m_travelTime = util::to_float(it.second);
	}
	m_particleNodes.resize(pSystem.GetMaxParticleCount(), 0);
}
void CParticleOperatorTrail::Simulate(CParticle &particle, double, float strength)
{
	auto vel = particle.GetVelocity();
	auto nodeIdx = particle.GetIndex();
	auto curNode = m_particleNodes[nodeIdx];
	auto nextNode = curNode + 1;
	auto posSrc = GetParticleSystem().GetNodePosition(curNode);
	auto posDst = GetParticleSystem().GetNodePosition(nextNode);
	auto dir = posDst - posSrc;
	uvec::normalize(&dir);

	//vel += dir *m_travelTime *static_cast<float>(tDelta);
	//particle.SetVelocity(vel);
	{
		auto &p0 = posSrc;
		auto &p1 = posDst;
		auto p2 = GetParticleSystem().GetNodePosition(nextNode + 1);
		auto p3 = GetParticleSystem().GetNodePosition(nextNode + 2);

		auto curveTightness = 1.f;
		auto s = particle.GetTimeAlive() / m_travelTime;
		auto t1 = curveTightness * (p2 - p0);
		auto t2 = curveTightness * (p3 - p1);

		auto hTransform = Vector4(2.f * powf(s, 3.f) - 3.f * powf(s, 2.f) + 1.f, -2.f * powf(s, 3.f) + 3.f * powf(s, 2.f), powf(s, 3.f) - 2.f * powf(s, 2.f) + s, powf(s, 3.f) - powf(s, 2.f));
		auto p = Vector3(hTransform[0] * p1.x + hTransform[1] * p2.x + hTransform[2] * t1.x + hTransform[3] * t2.x, hTransform[0] * p1.y + hTransform[1] * p2.y + hTransform[2] * t1.y + hTransform[3] * t2.y,
		  hTransform[0] * p1.z + hTransform[1] * p2.z + hTransform[2] * t1.z + hTransform[3] * t2.z);
		/*local P1 = points[i]
		local P0 = (i > 1) and points[i -1] or P1
		local P2 = points[i +1]
		local P3 = (i < (numPoints -2)) and points[i +2] or P2
		for s=0.0,1.0,0.1 do
			local T1 = curveTightness *(P2 -P0)
			local T2 = curveTightness *(P3 -P1)

			local h1 = 2 *s ^3 -3 *s ^2 +1
			local h2 = -2 *s ^3 +3 *s ^2
			local h3 = s ^3 -2 *s ^2 +s
			local h4 = s ^3 -s ^2
			local p = Vector(
				h1 *P1.x +h2 *P2.x +h3 *T1.x +h4 *T2.x,
				h1 *P1.y +h2 *P2.y +h3 *T1.y +h4 *T2.y,
				h1 *P1.z +h2 *P2.z +h3 *T1.z +h4 *T2.z
			)
			debugrenderer.draw_line(pPrev,p,Color(s *255.0,(1.0 -s) *255.0,0,255),12)
			pPrev = p
		end*/
		//auto pos = posSrc +(posDst -posSrc) *(particle.GetTimeAlive() /m_travelTime);
		particle.SetPosition(p);
	}
}
