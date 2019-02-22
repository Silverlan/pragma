#ifndef __C_SHADER_OCCLUSION_H__
#define __C_SHADER_OCCLUSION_H__
// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"
namespace Shader
{
	class DLLCLIENT Occlusion
		: public Base3D
	{
	public:
		Occlusion();
		using Base3D::BeginDraw;
		void PrepareBatchRendering(unsigned int indexBuffer);
		void Render(unsigned int vertexBuffer,unsigned int vertCount);
		void EndBatchRendering();
		enum class DLLCLIENT Location : uint32_t
		{
			Vertex = 0
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Vertex = 0,

			Camera = 0
		};
		enum class DLLCLIENT DescSet : uint32_t
		{
			Camera = 0
		};
	protected:
		virtual bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline) override;
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void InitializeAttachments(std::vector<vk::PipelineColorBlendAttachmentState> &attachments) override;
	};
};
#endif
#endif