// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "buffers/prosper_buffer.hpp"
#include "pragma/entities/environment/effects/env_particle_system.h"
#include <pragma/types.hpp>

export module pragma.client:particle_system.renderer_rotational_buffer;

export namespace pragma::ecs {class CParticleSystemComponent;}
export class DLLCLIENT CParticleRendererRotationalBuffer {
  public:
	CParticleRendererRotationalBuffer() = default;
	void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem);
	virtual bool Update();
	const std::shared_ptr<prosper::IBuffer> &GetBuffer() const;
	void SetRotationAlignVelocity(bool b);
	bool ShouldRotationAlignVelocity() const;
  protected:
	uint64_t m_lastFrameUpdate = std::numeric_limits<uint64_t>::max();
	std::shared_ptr<prosper::IBuffer> m_rotBuffer = nullptr;
	std::vector<Quat> m_rotations;
	pragma::ComponentHandle<pragma::ecs::CParticleSystemComponent> m_hParticleSystem = {};
	bool m_bAlignVelocity = false;
};
