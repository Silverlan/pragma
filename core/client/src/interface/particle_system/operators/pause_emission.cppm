// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.operator_pause_emission;

export import :entities.components.particle_system;
export import :particle_system.modifier;

export namespace pragma::pts {
	class DLLCLIENT CParticleOperatorPauseEmissionBase : public CParticleOperator {
	  public:
		virtual void Simulate(double tDelta) override;
		virtual void OnParticleSystemStarted() override;
	  protected:
		CParticleOperatorPauseEmissionBase() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual ecs::CParticleSystemComponent *GetTargetParticleSystem() = 0;
	  private:
		enum class State : uint32_t { Initial = 0u, Paused, Unpaused };
		float m_fStart = 0.f;
		float m_fEnd = 0.f;
		State m_state = State::Initial;
	};

	/////////////////////

	class DLLCLIENT CParticleOperatorPauseEmission : public CParticleOperatorPauseEmissionBase {
	  public:
		CParticleOperatorPauseEmission() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual ecs::CParticleSystemComponent *GetTargetParticleSystem() override;
	};

	/////////////////////

	class DLLCLIENT CParticleOperatorPauseChildEmission : public CParticleOperatorPauseEmissionBase {
	  public:
		CParticleOperatorPauseChildEmission() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual ecs::CParticleSystemComponent *GetTargetParticleSystem() override;
	  private:
		util::WeakHandle<ecs::CParticleSystemComponent> m_hChildSystem = {};
	};
};

void pragma::pts::CParticleOperatorPauseEmissionBase::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		if(key == "pause_start")
			m_fStart = util::to_float(pair.second);
		else if(key == "pause_end")
			m_fEnd = util::to_float(pair.second);
	}
	static_cast<ecs::CParticleSystemComponent &>(pSystem).SetAlwaysSimulate(true); // Required, otherwise Simulate() might not get called
}
void pragma::pts::CParticleOperatorPauseEmissionBase::OnParticleSystemStarted()
{
	CParticleOperator::OnParticleSystemStarted();
	Simulate(0.0);
}
void pragma::pts::CParticleOperatorPauseEmissionBase::Simulate(double tDelta)
{
	CParticleOperator::Simulate(tDelta);
	auto *ps = GetTargetParticleSystem();
	if(ps == nullptr)
		return;
	if(m_fEnd <= m_fStart)
		return;
	auto t = ps->GetLifeTime();
	switch(m_state) {
	case State::Initial:
		if(t < m_fStart)
			return;
		ps->PauseEmission();
		m_state = State::Paused;
		break;
	case State::Paused:
		if(t < m_fEnd)
			return;
		ps->ResumeEmission();
		m_state = State::Unpaused;
		break;
	case State::Unpaused:
		// Complete
		break;
	}
}

/////////////////////

void pragma::pts::CParticleOperatorPauseEmission::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) { CParticleOperatorPauseEmissionBase::Initialize(pSystem, values); }
pragma::ecs::CParticleSystemComponent *pragma::pts::CParticleOperatorPauseEmission::GetTargetParticleSystem() { return &GetParticleSystem(); }

/////////////////////

void pragma::pts::CParticleOperatorPauseChildEmission::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorPauseEmissionBase::Initialize(pSystem, values);
	std::string childName;
	for(auto &pair : values) {
		auto key = pair.first;
		string::to_lower(key);
		if(key == "name")
			childName = pair.second;
	}
	auto &children = GetParticleSystem().GetChildren();
	auto it = std::find_if(children.begin(), children.end(), [&childName](const ecs::CParticleSystemComponent::ChildData &hSystem) { return hSystem.child.valid() && string::match(childName, hSystem.child.get()->GetParticleSystemName()); });
	if(it == children.end())
		return;
	m_hChildSystem = it->child;
}
pragma::ecs::CParticleSystemComponent *pragma::pts::CParticleOperatorPauseChildEmission::GetTargetParticleSystem() { return m_hChildSystem.get(); }
