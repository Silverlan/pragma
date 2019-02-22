#ifndef __C_SHADER_REFRACTION_HPP__
#define __C_SHADER_REFRACTION_HPP__
// prosper TODO
#if 0
#include "shadersystem.h"
#include "pragma/rendering/shaders/c_shader_screen_clip.hpp"

namespace Shader
{
	class DLLCLIENT Refraction
		: public ScreenClip
	{
	public:
		enum class DLLCLIENT DescSet : std::underlying_type_t<ScreenClip::DescSet>
		{
			Texture = umath::to_integral(ScreenClip::DescSet::Texture),
			RefractionMap =  Texture +1,
			Time = RefractionMap +1
		};
		enum class DLLCLIENT Binding : std::underlying_type_t<ScreenClip::Binding>
		{
			Texture = 0,
			RefractionMap = 1,
			Time = 1
		};

		Refraction();
		Refraction(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		void BindMaterial(const Vulkan::CommandBufferObject *drawCmd,const CMaterial &mat);
		using ScreenClip::BeginDraw;
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializeRenderPasses() override;
		virtual bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline) override;
	};
};
#endif
#endif
