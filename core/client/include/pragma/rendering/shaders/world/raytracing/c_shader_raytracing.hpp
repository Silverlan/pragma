#ifndef __C_SHADER_RAYTRACING_HPP__
#define __C_SHADER_RAYTRACING_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderRayTracing
		: public prosper::ShaderCompute
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_IMAGE;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_BUFFERS;

#pragma pack(push,1)
		struct PushConstants
		{
			uint32_t numTris;
		};
#pragma pack(pop)

		ShaderRayTracing(prosper::Context &context,const std::string &identifier);
		bool Compute(Anvil::DescriptorSet &descSetImage,Anvil::DescriptorSet &descSetBuffers,uint32_t workGroupsX,uint32_t workGroupsY);

		void Test();
		bool ComputeTest();
	protected:
		virtual void InitializeComputePipeline(Anvil::ComputePipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
