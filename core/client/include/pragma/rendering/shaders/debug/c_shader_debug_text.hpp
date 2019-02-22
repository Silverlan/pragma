#ifndef __C_SHADER_DEBUG_TEXT_HPP__
#define __C_SHADER_DEBUG_TEXT_HPP__

#include "pragma/rendering/shaders/debug/c_shader_debug.hpp"

namespace pragma
{
	class DLLCLIENT ShaderDebugText
		: public ShaderDebug
	{
	public:
		ShaderDebugText(prosper::Context &context,const std::string &identifier);

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		bool Draw(Anvil::Buffer &vertexBuffer,uint32_t vertexCount,Anvil::DescriptorSet &descSetTexture,const Mat4 &mvp=umat::identity(),const Vector4 &color=Vector4(1.f,1.f,1.f,1.f));
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};


/*#include "pragma/rendering/shaders/debug/c_shader_debug.h"

namespace Shader
{
	class DLLCLIENT DebugText
		: public Debug
	{
	private:
		using Debug::Draw;
	protected:
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
	public:
		DebugText();
		void Draw(const Mat4 &matModel,Vulkan::DescriptorSetObject *descSet,Vulkan::BufferObject *vertexBuffer,uint32_t vertexCount,const Vulkan::DescriptorSet &texDescSet);
	};
};*/ // prosper TODO

#endif
