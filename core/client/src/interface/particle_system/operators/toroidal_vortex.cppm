// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_toroidal_vortex;

export import :particle_system.operator_world_base;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorToroidalVortex : public CParticleOperatorWorldBase {
	  public:
		CParticleOperatorToroidalVortex() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
		virtual void Simulate(double tDelta) override;
	  private:
		Vector3 m_vAxis = {0.f, 1.f, 0.f};
		float m_fHeight = 1.f;
		float m_fRadius = 1.f;
		float m_fStrength = 2.f;
		float m_fDivergence = 0.f;

		float m_dtStrength = 0.f;
		Vector3 m_dtOrigin = {};
		Vector3 m_dtAxis = {};
		Quat m_dtRotation = uquat::identity();
	};
}

void pragma::pts::CParticleOperatorToroidalVortex::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
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
		else if(key == "height")
			m_fHeight = util::to_float(it->second);
		else if(key == "radius")
			m_fRadius = util::to_float(it->second);
	}
}
void pragma::pts::CParticleOperatorToroidalVortex::Simulate(double tDelta)
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
void pragma::pts::CParticleOperatorToroidalVortex::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperatorWorldBase::Simulate(particle, tDelta, strength);
	// cross product of ring axis and particle position is tangent
	auto pos = particle.GetPosition() - m_dtOrigin;
	auto tangent = uvec::cross(m_dtAxis, pos);
	auto l = uvec::length(tangent);
	const auto EPSILON = 0.0001f;
	if(l < EPSILON)
		return; // particle is on the axis
	tangent *= 1.f / l;

	// cross product of tangent and axis is direction from axis to position
	auto v = uvec::cross(tangent, m_dtAxis);

	// find vector from closest point on ring to position
	v *= m_fRadius;
	v += m_dtAxis * m_fHeight;
	v -= pos;
	l = uvec::length(v);
	if(l < EPSILON)
		return; // particle is on the ring
	v *= 1.f / l;

	// compute the rotation angle
	auto rot = uquat::create(tangent, m_fDivergence);

	// cross product of vector and tangent is direction
	v = uvec::cross(v, tangent) * static_cast<float>(tDelta);
	uvec::rotate(&v, rot);
	particle.SetVelocity(particle.GetVelocity() + v);
}
