/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_OPERATOR_PAUSE_EMISSION_HPP__
#define __C_PARTICLE_OPERATOR_PAUSE_EMISSION_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_time.hpp"

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

#endif
