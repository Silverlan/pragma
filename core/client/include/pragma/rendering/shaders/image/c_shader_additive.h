#ifndef __C_SHADER_ADDITIVE_H__
#define __C_SHADER_ADDITIVE_H__
// prosper TODO
#if 0
#include "shadersystem.h"
#include "shader_screen.h"

namespace Shader
{
	class DLLCLIENT Additive
		: public Screen
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			Texture = 0,
			TextureAdd = 0
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Texture = 0,
			TextureAdd = 1
		};
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	public:
		Additive();
		bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,float addScale);
	};
};
#endif
#endif