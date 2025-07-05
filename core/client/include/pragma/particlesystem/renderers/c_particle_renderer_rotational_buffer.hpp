// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_PARTICLE_RENDERER_ROTATIONAL_BUFFER_HPP__
#define __C_PARTICLE_RENDERER_ROTATIONAL_BUFFER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include <pragma/types.hpp>

namespace prosper {
	class Buffer;
};
class DLLCLIENT CParticleRendererRotationalBuffer {
  public:
	CParticleRendererRotationalBuffer() = default;
	void Initialize(pragma::CParticleSystemComponent &pSystem);
	virtual bool Update();
	const std::shared_ptr<prosper::IBuffer> &GetBuffer() const;
	void SetRotationAlignVelocity(bool b);
	bool ShouldRotationAlignVelocity() const;
  protected:
	uint64_t m_lastFrameUpdate = std::numeric_limits<uint64_t>::max();
	std::shared_ptr<prosper::IBuffer> m_rotBuffer = nullptr;
	std::vector<Quat> m_rotations;
	pragma::ComponentHandle<pragma::CParticleSystemComponent> m_hParticleSystem = {};
	bool m_bAlignVelocity = false;
};

#endif
