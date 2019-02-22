#ifndef __C_SHADER_SCREEN_CLIP_HPP__
#define __C_SHADER_SCREEN_CLIP_HPP__
// prosper TODO
#if 0
#include "shadersystem.h"
#include <shader_screen.h>

namespace Shader
{
	class DLLCLIENT ScreenClip
		: public Screen
	{
	public:
		ScreenClip();
		ScreenClip(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		virtual void Draw(const Vulkan::DescriptorSet &descTexture,const Mat4 &mvp,const Vector4 &clipPlane);
		virtual void BindScene(const Vulkan::CommandBufferObject *drawCmd,const Scene &scene);
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	};
};
#endif
#endif
