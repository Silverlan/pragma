#ifndef __C_SHADER_DEBUG_NORMALS_H__
#define __C_SHADER_DEBUG_NORMALS_H__
// prosper TODO
#if 0
#include "pragma/rendering/shaders/c_shader.h"

namespace Shader
{
	class DLLCLIENT DebugNormals
		: public TexturedBase3D
	{
	public:
		enum class DLLCLIENT RenderMode : uint32_t
		{
			Normals = 0,
			NormalsAndTangents = 1,
			FaceNormals = 2,
			Tangents = 3,
			BiTangents = 4
		};
		enum class DLLCLIENT DescSet : uint32_t
		{
			Instance = 0,
			BoneMatrix = Instance,
			Camera = BoneMatrix +1
		};
		enum class DLLCLIENT Location : uint32_t
		{
			Vertex = 2,
			Normal = 3,
			Tangent = 4,
			BiTangent = 5,
			BoneWeightId = 0,
			BoneWeight = 1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Vertex = 0,
			Normal = 0,
			Tangent = 0,
			BiTangent = 0,
			BoneWeightId = 1,
			BoneWeight = 1,

			Instance = 0,
			BoneMatrix = Instance +1,
			Camera = 0
		};
		static const uint32_t MAX_INSTANCE_COUNT;
		static const uint32_t PUSH_CONSTANT_COUNT;
	protected:
		using TexturedBase3D::BindLights;
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
		virtual bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline) override;
	public:
		DebugNormals();
		bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,RenderMode mode);
		bool BeginDraw(RenderMode mode);
	};
};
#endif
#endif