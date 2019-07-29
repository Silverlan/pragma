#ifndef __C_SSAO_HPP__
#define __C_SSAO_HPP__

#include "pragma/clientdefinitions.h"
#include <memory>
#include <image/prosper_texture.hpp>

namespace prosper {class Shader; class Context; class RenderTarget; class DescriptorSetGroup;};

#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLCLIENT SSAOInfo
{
	bool Initialize(
		prosper::Context &context,uint32_t width,uint32_t height,Anvil::SampleCountFlagBits samples,
		const std::shared_ptr<prosper::Texture> &texNorm,const std::shared_ptr<prosper::Texture> &texDepth
	);
	void Clear();
	std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
	std::shared_ptr<prosper::RenderTarget> renderTargetBlur = nullptr;
	std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupPrepass = nullptr;
	std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupOcclusion = nullptr;

	::util::WeakHandle<prosper::Shader> shader = {};
	::util::WeakHandle<prosper::Shader> shaderBlur = {};
	prosper::Shader *GetSSAOShader() const;
	prosper::Shader *GetSSAOBlurShader() const;
	/*void Initialize(const Vulkan::Context &context,uint32_t width,uint32_t height,Anvil::SampleCountFlagBits sampleCount,const Vulkan::Texture &texNorm,const Vulkan::Texture &texDepth);
	void Clear();
	Vulkan::RenderPass renderPass = nullptr;
	Vulkan::Framebuffer framebuffer = nullptr;
	Vulkan::DescriptorSet descSetNormalDepthBuffer = nullptr;

	Vulkan::RenderTarget rtOcclusion = nullptr;
	Vulkan::DescriptorSet descSetOcclusion = nullptr;

	Vulkan::RenderTarget rtOcclusionBlur = nullptr;*/ // prosper TODO
};
#pragma warning(pop)

#endif
