#ifndef __C_PARTICLE_MOD_SPRITE_H__
#define __C_PARTICLE_MOD_SPRITE_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include <sharedutils/util_weak_handle.hpp>

class CParticleRendererRotationalBuffer;
namespace pragma {class CLightComponent;};
class DLLCLIENT CParticleRendererSprite
	: public CParticleRenderer
{
public:
	CParticleRendererSprite(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual void Render(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Scene &scene,bool bloom) override;
	virtual void RenderShadow(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Scene &scene,pragma::CLightComponent &light,uint32_t layerId=0) override;
	virtual void PostSimulate(double tDelta) override;
private:
	::util::WeakHandle<prosper::Shader> m_shader = {};
	std::unique_ptr<CParticleRendererRotationalBuffer> m_rotationalBuffer = nullptr;
	bool m_bPlanarRotation = true;
};

#endif