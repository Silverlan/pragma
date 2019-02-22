#ifndef __C_SHADER_SSAO_HPP__
#define __C_SHADER_SSAO_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace pragma
{
	class DLLCLIENT ShaderSSAO
		: public prosper::ShaderBaseImageProcessing
	{
	public:
		static Anvil::Format RENDER_PASS_FORMAT;

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_PREPASS;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_NOISE_TEXTURE;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_SAMPLE_BUFFER;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;

		enum class PrepassBinding : uint32_t
		{
			NormalBuffer = 0u,
			DepthBuffer
		};

#pragma pack(push,1)
		struct PushConstants
		{
			std::array<uint32_t,2> renderTargetDimensions;
		};
#pragma pack(pop)

		ShaderSSAO(prosper::Context &context,const std::string &identifier);
		bool Draw(const Scene &scene,Anvil::DescriptorSet &descSetPrepass,const std::array<uint32_t,2> &renderTargetDimensions);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void OnPipelineInitialized(uint32_t pipelineIdx) override;
	private:
		std::shared_ptr<prosper::Buffer> m_kernelBuffer = nullptr;
		std::shared_ptr<prosper::Texture> m_noiseTexture = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupKernel = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupTexture = nullptr;
	};
};

#endif