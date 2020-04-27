/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/operators/c_particle_operator_world_base.hpp"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

void CParticleOperatorWorldBase::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	CParticleOperator::Initialize(pSystem,values);
	for(auto it=values.begin();it!=values.end();it++)
	{
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "rotate_with_emitter")
			m_bRotateWithEmitter = util::to_boolean(it->second);
	}
}

bool CParticleOperatorWorldBase::ShouldRotateWithEmitter() const {return m_bRotateWithEmitter;}
