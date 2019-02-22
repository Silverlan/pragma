#ifndef __C_SHADER_FORWARDP_LIGHT_INDEXING_HPP__
#define __C_SHADER_FORWARDP_LIGHT_INDEXING_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderForwardPLightIndexing
		: public prosper::ShaderCompute
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_VISIBLE_LIGHT;

#pragma pack(push,1)
		struct PushConstants
		{
			uint32_t tileCount;
		};
#pragma pack(pop)

		ShaderForwardPLightIndexing(prosper::Context &context,const std::string &identifier);
		bool Compute(Anvil::DescriptorSet &descSetLights,uint32_t tileCount);
	protected:
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "shadersystem.h"

namespace Shader
{
	class DLLCLIENT ForwardPLightIndexing
		: public Base
	{
	public:
		ForwardPLightIndexing();
		void Compute(const Vulkan::CommandBuffer &computeCmd,const Vulkan::DescriptorSet &descSetLights,uint32_t tileCount); // Currently no barriers in place; Returned color might be from a previous call
		static Vulkan::DescriptorSet CreateLightDescriptorSet();

		enum class DescSet : uint32_t
		{
			TileVisLightIndexBuffer = 0,
			VisLightIndexBuffer = TileVisLightIndexBuffer
		};
		enum class Binding : uint32_t
		{
			TileVisLightIndexBuffer = 0,
			VisLightIndexBuffer = TileVisLightIndexBuffer +1
		};
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	};
};
#endif
#endif
