// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_cylindrical_vortex;

export import :particle_system.operator_world_base;

export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorCylindricalVortex : public CParticleOperatorWorldBase {
	  public:
		CParticleOperatorCylindricalVortex() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
		virtual void Simulate(double tDelta) override;
	  private:
		Vector3 m_vAxis = {0.f, 1.f, 0.f};
		float m_fStrength = 2.f;
		float m_fDivergence = 0.f;

		float m_dtStrength = 0.f;
		Vector3 m_dtOrigin = {};
		Vector3 m_dtAxis = {};
		Quat m_dtRotation = uquat::identity();
	};
}

void pragma::pts::CParticleOperatorCylindricalVortex::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorWorldBase::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		string::to_lower(key);
		if(key == "axis")
			m_vAxis = uvec::get_normal(uvec::create(it->second));
		else if(key == "strength")
			m_fStrength = util::to_float(it->second);
		else if(key == "divergence")
			m_fDivergence = util::to_float(it->second);
	}
}
void pragma::pts::CParticleOperatorCylindricalVortex::Simulate(double tDelta)
{
	CParticleOperatorWorldBase::Simulate(tDelta);
	m_dtStrength = m_fStrength * tDelta;

	// transform the origin and axis into particle space
	auto &ps = GetParticleSystem();
	m_dtOrigin = ps.PointToParticleSpace(Vector3 {}, true);
	m_dtAxis = ps.DirectionToParticleSpace(m_vAxis, ShouldRotateWithEmitter());

	// find divergence rotation
	m_dtRotation = uquat::create(m_dtAxis, -m_fDivergence);
}
void pragma::pts::CParticleOperatorCylindricalVortex::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperatorWorldBase::Simulate(particle, tDelta, strength);
	// cross product of vortex axis and relative position is direction
	auto v = uvec::cross(m_dtAxis, particle.GetPosition() - m_dtOrigin);
	auto l = uvec::length(v);
	const auto EPSILON = 0.0001f;
	if(l < EPSILON)
		return; // particle is on the axis
	// normalize direction, scale by delta, rotate, add to velocity
	v *= m_dtStrength / l;
	uvec::rotate(&v, m_dtRotation);
	particle.SetVelocity(particle.GetVelocity() + v);
}
