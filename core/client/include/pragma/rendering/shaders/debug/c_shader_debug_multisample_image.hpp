#ifndef __C_SHADER_DEBUG_MULTISAMPLE_IMAGE_HPP__
#define __C_SHADER_DEBUG_MULTISAMPLE_IMAGE_HPP__
// prosper TODO
#if 0
#include "shadersystem.h"
#include "shader_screen.h"

namespace Shader
{
	class DLLCLIENT DebugMultisampleImage
		: public Screen
	{
	public:
		DebugMultisampleImage(const std::string &identifier,const std::string &vsShader,const std::string &fsShader);
		DebugMultisampleImage();

		enum class DescSet : uint32_t
		{
			Texture = 0u
		};

		enum class Binding : uint32_t
		{
			Texture = 0u
		};
		void Draw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descTexture,uint32_t width,uint32_t height,uint32_t sampleCount);
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	};
	class DLLCLIENT DebugMultisampleDepth
		: public DebugMultisampleImage
	{
	public:
		DebugMultisampleDepth();
	};
};
#endif
#endif
