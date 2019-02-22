#ifndef __C_SHADER_CALC_IMAGE_COLOR_HPP__
#define __C_SHADER_CALC_IMAGE_COLOR_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderCalcImageColor
		: public prosper::ShaderCompute
	{
	public:
#pragma pack(push,1)
		struct PushConstants
		{
			int32_t sampleCount;
		};
#pragma pack(pop)

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_COLOR;

		ShaderCalcImageColor(prosper::Context &context,const std::string &identifier);
		bool Compute(Anvil::DescriptorSet &descSetTexture,Anvil::DescriptorSet &descSetColor,uint32_t sampleCount);
	protected:
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "shadersystem.h"

namespace Shader
{
	class DLLCLIENT CalcImageColor
		: public Base
	{
	public:
		static const uint32_t SHADER_DESC_SET_TEXTURE;
		static const uint32_t SHADER_DESC_SET_COLOR;
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	public:
		CalcImageColor();
		void Compute(const Vulkan::DescriptorSet &descColor,const Vulkan::DescriptorSet &descTexture,uint32_t sampleCount); // Currently no barriers in place; Returned color might be from a previous call
		static Vulkan::DescriptorSet CreateTextureDescriptorSet();
		static Vulkan::DescriptorSet CreateColorDescriptorSet();
	};
};
#endif
#endif