// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:particle_system.operator_texture_scrolling;

export import :entities.components.particle_system;
export import :particle_system.modifier;

export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorTextureScrolling : public CParticleOperator {
	public:
		CParticleOperatorTextureScrolling() = default;
		virtual void Simulate(pragma::pts::CParticle &particle, double, float strength) override;
		virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleCreated(pragma::pts::CParticle &particle) override;
	private:
		void SetFrameOffset(pragma::pts::CParticle &particle, Vector2 uv);
		float m_fHorizontalSpeed = 0.f;
		float m_fVerticalSpeed = 0.f;
	};
}

void pragma::pts::CParticleOperatorTextureScrolling::Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "horizontal_speed")
			m_fHorizontalSpeed = util::to_float(pair.second);
		else if(key == "vertical_speed")
			m_fVerticalSpeed = util::to_float(pair.second);
	}
	static_cast<pragma::ecs::CParticleSystemComponent &>(pSystem).SetTextureScrollingEnabled(true);
}
void pragma::pts::CParticleOperatorTextureScrolling::SetFrameOffset(pragma::pts::CParticle &particle, Vector2 uv)
{
	umath::normalize_uv_coordinates(uv);

	auto frameOffset = glm::packHalf2x16(uv);
	particle.SetFrameOffset(reinterpret_cast<float &>(frameOffset));
}
void pragma::pts::CParticleOperatorTextureScrolling::OnParticleCreated(pragma::pts::CParticle &particle) { SetFrameOffset(particle, {}); }
void pragma::pts::CParticleOperatorTextureScrolling::Simulate(pragma::pts::CParticle &particle, double dt, float strength)
{
	auto offsetH = m_fHorizontalSpeed * dt;
	auto offsetV = m_fVerticalSpeed * dt;

	// Horizontal and vertical scroll offsets are encoded as 16 bit floating point values
	// as the particle's frame offset. Particles cannot be animated and have texture scrolling
	// at the same time!
	auto frameOffset = particle.GetFrameOffset();
	auto uv = glm::unpackHalf2x16(reinterpret_cast<uint32_t &>(frameOffset));
	uv.x += offsetH;
	uv.y += offsetV;
	umath::normalize_uv_coordinates(uv);

	SetFrameOffset(particle, uv);
}
