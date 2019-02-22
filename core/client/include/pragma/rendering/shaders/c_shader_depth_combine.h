#ifndef __C_SHADER_DEPTH_COMBINE_H__
#define __C_SHADER_DEPTH_COMBINE_H__
// prosper TODO
#if 0
#include "shadersystem.h"
#include "shader_screen.h"

namespace Shader
{
	class DLLCLIENT DepthCombine
		: public Screen
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			DepthMap0 = 0,
			DepthMap1 = 0
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			DepthMap0 = 0,
			DepthMap1 = 1
		};
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void InitializeRenderPasses() override;
	public:
		DepthCombine();
	};
};
#endif
#endif