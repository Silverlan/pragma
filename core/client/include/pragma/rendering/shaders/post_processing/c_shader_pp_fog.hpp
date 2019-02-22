#ifndef __C_SHADER_PP_FOG_HPP__
#define __C_SHADER_PP_FOG_HPP__

#include "pragma/rendering/shaders/post_processing/c_shader_pp_base.hpp"

namespace pragma
{
	class DLLCLIENT ShaderPPFog
		: public ShaderPPBase
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_DEPTH_BUFFER;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_FOG;

#pragma pack(push,1)
		struct DLLCLIENT Fog
		{
			enum class DLLCLIENT Type : uint32_t
			{
				Linear = 0,
				Exponential,
				Exponential2
			};
			enum class DLLCLIENT Flag : uint32_t
			{
				None = 0,
				Enabled = 1
			};
			Vector4 color = {0.f,0.f,0.f,0.f};
			float start = 0.f;
			float end = 0.f;
			float density = 0.f;
			Type type = Type::Linear;
			Flag flags = Flag::None;
		};
#pragma pack(pop)

		ShaderPPFog(prosper::Context &context,const std::string &identifier);
		bool Draw(Anvil::DescriptorSet &descSetTexture,Anvil::DescriptorSet &descSetDepth,Anvil::DescriptorSet &descSetCamera,Anvil::DescriptorSet &descSetFog);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "shadersystem.h"
#include "shader_screen.h"

namespace Shader
{
	class DLLCLIENT PPFog
		: public Screen
	{
	public:
		PPFog();

		enum class DescSet : uint32_t
		{
			Texture = 0u,

			DepthBuffer = Texture +1u,
			
			RenderSettings = DepthBuffer +1u,

			Fog = RenderSettings +1u
		};

		enum class Binding : uint32_t
		{
			Texture = 0u,

			DepthBuffer = 0u,

			RenderSettings = 1u,

			Fog = 0u,
		};
		void Draw(Vulkan::CommandBufferObject *cmdBuffer,const Vulkan::DescriptorSet &descTexture,const Vulkan::DescriptorSet &descDepth,const Vulkan::DescriptorSet &descFog);
		void BindScene(const Vulkan::CommandBufferObject *drawCmd,const Scene &scene);
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	};
};
#endif
#endif
