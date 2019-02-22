#ifndef __C_SHADER_HDR_HPP__
#define __C_SHADER_HDR_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace pragma
{
	class DLLCLIENT ShaderHDR
		: public prosper::ShaderBaseImageProcessing
	{
	public:
#pragma pack(push,1)
		struct PushConstants
		{
			float exposure;
		};
#pragma pack(pop)

		ShaderHDR(prosper::Context &context,const std::string &identifier);
		bool Draw(Anvil::DescriptorSet &descSetTexture,float exposure);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	private:
		bool Draw(Anvil::DescriptorSet &descSetTexture)=delete;
	};
};

// prosper TODO
#if 0
#include "shadersystem.h"
#include "shader_screen.h"

namespace Shader
{
	class DLLCLIENT HDR
		: public Screen
	{
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	public:
		HDR();
		bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,float exposure);
	};
};
#endif
#endif
