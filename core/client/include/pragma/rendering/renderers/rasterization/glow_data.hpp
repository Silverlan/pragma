#ifndef __GLOW_DATA_HPP__
#define __GLOW_DATA_HPP__

#include "pragma/clientdefinitions.h"
#include <cinttypes>
#include <vector>
#include <memory>
#include <sharedutils/util_weak_handle.hpp>
#include <sharedutils/functioncallback.h>

namespace prosper {class RenderTarget; class BlurSet; class Shader;};
namespace pragma {class CParticleSystemComponent;};
namespace pragma::rendering
{
	class HDRData;
	class DLLCLIENT GlowData
	{
	public:
		GlowData();
		~GlowData();
		GlowData(const GlowData&)=delete;
		GlowData &operator=(const GlowData&)=delete;
		bool Initialize(uint32_t width,uint32_t height,const HDRData &hdrInfo);
		//Vulkan::DescriptorSet descSetAdditive; // prosper TODO
		util::WeakHandle<prosper::Shader> shader = {};
		std::vector<pragma::CParticleSystemComponent*> tmpBloomParticles;
		bool bGlowScheduled = false; // Glow meshes scheduled for this frame? (=tmpGlowMeshes isn't empty)
		std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
		std::shared_ptr<prosper::BlurSet> blurSet = nullptr;
	private:
		CallbackHandle m_cbReloadCommandBuffer;
	};
};

#endif
