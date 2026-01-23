// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_animation_playback;

export import :particle_system.modifier;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorAnimationPlayback : public CParticleOperator {
	  public:
		CParticleOperatorAnimationPlayback() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
	  private:
		float m_playbackSpeed = 1.f;
	};
}

void pragma::pts::CParticleOperatorAnimationPlayback::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		if(key == "playbackspeed")
			m_playbackSpeed = util::to_float(pair.second);
	}
}
void pragma::pts::CParticleOperatorAnimationPlayback::Simulate(CParticle &particle, double tDelta, float strength)
{
	CParticleOperator::Simulate(particle, tDelta, strength);
	particle.SetFrameOffset(fmodf(particle.GetFrameOffset() + tDelta * m_playbackSpeed, 1.f));
}
