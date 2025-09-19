// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <optional>

export module pragma.client.particle_system:initializer_initial_velocity;

export class DLLCLIENT CParticleInitializerInitialVelocity : public CParticleInitializer {
  public:
	CParticleInitializerInitialVelocity() = default;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
	float GetSpeed() const;
  private:
	Vector3 m_direction = {};
	float m_speed = 0.f;
	Vector3 m_spreadMin = {};
	Vector3 m_spreadMax = {};
	Vector3 m_velocityMin = {};
	Vector3 m_velocityMax = {};
};
