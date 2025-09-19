// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/base_transform_component.hpp"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

export module pragma.client.particle_system:operator_wind;

export class DLLCLIENT CParticleOperatorWind : public CParticleOperator {
  public:
	CParticleOperatorWind() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
	virtual void Simulate(double tDelta) override;
  private:
	bool m_bRotateWithEmitter = false;
	float m_fStrength = 2.f;
	Vector3 m_vDirection = {1.f, 0.f, 0.f};
	Vector3 m_vDelta = {};
};

REGISTER_PARTICLE_OPERATOR(wind, CParticleOperatorWind);

void CParticleOperatorWind::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		ustring::to_lower(key);
		if(key == "strength")
			m_fStrength = util::to_float(it->second);
		else if(key == "direction")
			m_vDirection = uvec::create(it->second);
		else if(key == "rotate_with_emitter")
			m_bRotateWithEmitter = util::to_boolean(it->second);
	}
}
void CParticleOperatorWind::Simulate(double tDelta)
{
	CParticleOperator::Simulate(tDelta);
	m_vDelta = m_vDirection * (m_fStrength * static_cast<float>(tDelta));
	if(m_bRotateWithEmitter) {
		auto pTrComponent = GetParticleSystem().GetEntity().GetTransformComponent();
		if(pTrComponent != nullptr)
			uvec::rotate(&m_vDelta, pTrComponent->GetRotation());
	}
}
void CParticleOperatorWind::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperator::Simulate(particle, tDelta, strength);
	particle.SetVelocity(particle.GetVelocity() + m_vDelta);
}
