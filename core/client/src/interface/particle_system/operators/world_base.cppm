// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <mathutil/umath.h>

export module pragma.client:particle_system.operator_world_base;

import :entities.components.particle_system;
import :particle_system.modifier;

export class DLLCLIENT CParticleOperatorWorldBase : public CParticleOperator {
  public:
	bool ShouldRotateWithEmitter() const;
  protected:
	CParticleOperatorWorldBase() = default;
	virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
  private:
	bool m_bRotateWithEmitter = false;
};

void CParticleOperatorWorldBase::Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "rotate_with_emitter")
			m_bRotateWithEmitter = util::to_boolean(it->second);
	}
}

bool CParticleOperatorWorldBase::ShouldRotateWithEmitter() const { return m_bRotateWithEmitter; }
