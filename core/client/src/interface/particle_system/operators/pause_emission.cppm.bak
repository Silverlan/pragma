// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <algorithm>

export module pragma.client.particle_system:operator_pause_emission;

export {
	class DLLCLIENT CParticleOperatorPauseEmissionBase : public CParticleOperator {
	public:
		virtual void Simulate(double tDelta) override;
		virtual void OnParticleSystemStarted() override;
	protected:
		CParticleOperatorPauseEmissionBase() = default;
		virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual pragma::CParticleSystemComponent *GetTargetParticleSystem() = 0;
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
		virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual pragma::CParticleSystemComponent *GetTargetParticleSystem() override;
	};

	/////////////////////

	class DLLCLIENT CParticleOperatorPauseChildEmission : public CParticleOperatorPauseEmissionBase {
	public:
		CParticleOperatorPauseChildEmission() = default;
		virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual pragma::CParticleSystemComponent *GetTargetParticleSystem() override;
	private:
		util::WeakHandle<pragma::CParticleSystemComponent> m_hChildSystem = {};
	};
};

REGISTER_PARTICLE_OPERATOR(pause_emission, CParticleOperatorPauseEmission);
REGISTER_PARTICLE_OPERATOR(pause_child_emission, CParticleOperatorPauseChildEmission);

void CParticleOperatorPauseEmissionBase::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperator::Initialize(pSystem, values);
	for(auto &pair : values) {
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "pause_start")
			m_fStart = util::to_float(pair.second);
		else if(key == "pause_end")
			m_fEnd = util::to_float(pair.second);
	}
	pSystem.SetAlwaysSimulate(true); // Required, otherwise Simulate() might not get called
}
void CParticleOperatorPauseEmissionBase::OnParticleSystemStarted()
{
	CParticleOperator::OnParticleSystemStarted();
	Simulate(0.0);
}
void CParticleOperatorPauseEmissionBase::Simulate(double tDelta)
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

void CParticleOperatorPauseEmission::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) { CParticleOperatorPauseEmissionBase::Initialize(pSystem, values); }
pragma::CParticleSystemComponent *CParticleOperatorPauseEmission::GetTargetParticleSystem() { return &GetParticleSystem(); }

/////////////////////

void CParticleOperatorPauseChildEmission::Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleOperatorPauseEmissionBase::Initialize(pSystem, values);
	std::string childName;
	for(auto &pair : values) {
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "name")
			childName = pair.second;
	}
	auto &children = GetParticleSystem().GetChildren();
	auto it = std::find_if(children.begin(), children.end(), [&childName](const pragma::CParticleSystemComponent::ChildData &hSystem) { return hSystem.child.valid() && ustring::match(childName, hSystem.child.get()->GetParticleSystemName()); });
	if(it == children.end())
		return;
	m_hChildSystem = it->child;
}
pragma::CParticleSystemComponent *CParticleOperatorPauseChildEmission::GetTargetParticleSystem() { return m_hChildSystem.get(); }
