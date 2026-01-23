// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_world_base;

export import :entities.components.particle_system;
export import :particle_system.modifier;

export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorWorldBase : public CParticleOperator {
	  public:
		bool ShouldRotateWithEmitter() const;
	  protected:
		CParticleOperatorWorldBase() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	  private:
		bool m_bRotateWithEmitter = false;
	};
}

void pragma::pts::CParticleOperatorWorldBase::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		string::to_lower(key);
		if(key == "rotate_with_emitter")
			m_bRotateWithEmitter = util::to_boolean(it->second);
	}
}

bool pragma::pts::CParticleOperatorWorldBase::ShouldRotateWithEmitter() const { return m_bRotateWithEmitter; }
