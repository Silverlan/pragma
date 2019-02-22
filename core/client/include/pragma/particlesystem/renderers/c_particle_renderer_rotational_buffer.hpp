#ifndef __C_PARTICLE_RENDERER_ROTATIONAL_BUFFER_HPP__
#define __C_PARTICLE_RENDERER_ROTATIONAL_BUFFER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"

namespace prosper {class Buffer;};
class DLLCLIENT CParticleRendererRotationalBuffer
{
public:
	CParticleRendererRotationalBuffer(pragma::CParticleSystemComponent &pSystem);
	virtual bool Update();
	const std::shared_ptr<prosper::Buffer> &GetBuffer() const;
	void SetRotationAlignVelocity(bool b);
	bool ShouldRotationAlignVelocity() const;
protected:
	uint64_t m_lastFrameUpdate = std::numeric_limits<uint64_t>::max();
	std::shared_ptr<prosper::Buffer> m_rotBuffer = nullptr;
	std::vector<Quat> m_rotations;
	util::WeakHandle<pragma::CParticleSystemComponent> m_hParticleSystem = {};
	bool m_bAlignVelocity = false;
};

#endif
