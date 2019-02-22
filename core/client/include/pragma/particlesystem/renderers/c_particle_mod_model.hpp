#ifndef __C_PARTICLE_MOD_MODEL_HPP__
#define __C_PARTICLE_MOD_MODEL_HPP__

#include "pragma/particlesystem/renderers/c_particle_renderer_rotational_buffer.hpp"
// TODO: Remove this
#if 0

class BaseEntity;
class CParticleRendererModel;
struct DLLCLIENT IParticleModelComponent
	: public pragma::ComponentModel::ICallback
{
	CParticleRendererModel &renderer;
	IParticleModelComponent(CParticleRendererModel &renderer);
	virtual Vector3 GetOrigin() override;
	virtual const Quat &GetOrientation() override;
	virtual const Vector3 &GetPosition() override;
	virtual const Vector3 &GetScale() override;
	virtual NetworkState *GetNetworkState() override;
	virtual void AnimEventEmitSound(const std::string &snd,ALSoundType type) override;
	virtual bool MaintainAnimation(pragma::ComponentModel::AnimationSlotInfo &animInfo,double dt) override;
	virtual bool ShouldUpdateBones() override;
};

///////////////////////
#endif

namespace pragma {class CAnimatedComponent; class CLightComponent;};
class DLLCLIENT CParticleRendererModel
	: public CParticleRenderer
{
public:
	CParticleRendererModel(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual ~CParticleRendererModel() override;
	virtual void Render(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Scene &scene,bool bloom) override;
	virtual void RenderShadow(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Scene &scene,pragma::CLightComponent &light,uint32_t layerId=0) override;
	virtual void PostSimulate(double tDelta) override;
	virtual void Initialize(CParticle &particle) override;

	bool IsAnimated() const;
protected:
	static std::size_t s_rendererCount;

	struct ParticleModelComponent
	{
		util::WeakHandle<pragma::CAnimatedComponent> animatedComponent;
		std::shared_ptr<prosper::DescriptorSetGroup> instanceDescSetGroupAnimated;
	};
	ParticleModelComponent &GetParticleComponent(uint32_t particleIdx);
	std::vector<ParticleModelComponent> m_particleComponents;

	std::string m_animation;
	util::WeakHandle<prosper::Shader> m_shader = {};
	uint64_t m_lastFrameUpdate = std::numeric_limits<uint64_t>::max();
	CParticleRendererRotationalBuffer m_rotationalBuffer;
	bool Update();
};

#endif
