// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_gravity;

export import :particle_system.operator_world_base;

import :engine;
import :game;

export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorGravity : public CParticleOperatorWorldBase {
	  public:
		CParticleOperatorGravity() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
		virtual void Simulate(double tDelta) override;
	  protected:
		float m_gravityScale = 1.f;
		Vector3 m_gravityForce = {0.f, -1.f, 0.f};
		bool m_bUseCustomGravityForce = false;

		Vector3 m_dtGravity = {};
	};
}

void pragma::pts::CParticleOperatorGravity::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorWorldBase::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		std::string key = it->first;
		string::to_lower(key);
		if(key == "effective_scale")
			m_gravityScale = util::to_float(it->second);
		else if(key == "acceleration") {
			m_bUseCustomGravityForce = true;
			m_gravityForce = uvec::create(it->second);
		}
	}
}
void pragma::pts::CParticleOperatorGravity::Simulate(double tDelta)
{
	CParticleOperatorWorldBase::Simulate(tDelta);
	if(m_bUseCustomGravityForce == false)
		return;
	m_dtGravity = GetParticleSystem().DirectionToParticleSpace(m_gravityForce * static_cast<float>(tDelta), ShouldRotateWithEmitter());
}
void pragma::pts::CParticleOperatorGravity::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperatorWorldBase::Simulate(particle, tDelta, strength);
	if(m_bUseCustomGravityForce) {
		particle.SetVelocity(particle.GetVelocity() + m_dtGravity);
		return;
	}
	auto &gravity = get_client_game()->GetGravity();
	auto &oldVel = particle.GetVelocity();
	particle.SetVelocity(oldVel + (m_bUseCustomGravityForce ? m_gravityForce : gravity) * m_gravityScale * static_cast<float>(tDelta));
}
