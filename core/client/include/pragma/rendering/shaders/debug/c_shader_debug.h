#ifndef __C_SHADER_DEBUG_H__
#define __C_SHADER_DEBUG_H__
// prosper TODO
#if 0
#include "pragma/rendering/shaders/c_shader.h"

namespace Shader
{
	class DLLCLIENT Debug
		: public Base3D
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			Instance = 0
		};
		enum class DLLCLIENT Location : uint32_t
		{
			Vertex = 0,
			Color = 1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Vertex = 0,
			Color = 1
		};
		static const uint32_t MAX_INSTANCE_COUNT;
	protected:
		Debug(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
		virtual void InitializeDynamicStates(std::vector<vk::DynamicState> &states) override;
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
		virtual uint32_t GetVertexStride() const;
		using Base3D::BeginDraw;
	public:
		Debug();
		bool BeginDraw(Camera &cam);
		void Draw(const Mat4 &matModel,Vulkan::DescriptorSetObject *descSet,Vulkan::BufferObject *vertexBuffer,uint32_t vertexCount,Vulkan::BufferObject *colorBuffer=nullptr);
		void Draw(Vulkan::DescriptorSetObject *descSet,Vulkan::BufferObject *vertexBuffer,uint32_t vertexCount,Vulkan::BufferObject *colorBuffer=nullptr);
	};
	class DLLCLIENT DebugLine
		: public Debug
	{
	protected:
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
	public:
		using Debug::Debug;
	};
	class DLLCLIENT DebugTriangleLine
		: public DebugLine
	{
	protected:
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
	public:
		using DebugLine::DebugLine;
	};
	class DLLCLIENT DebugLineStrip
		: public DebugLine
	{
	protected:
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
	public:
		using DebugLine::DebugLine;
	};
	class DLLCLIENT DebugPoint
		: public Debug
	{
	protected:
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
	public:
		using Debug::Debug;
	};
	class DLLCLIENT DebugPointVertex
		: public DebugPoint
	{
	protected:
		virtual uint32_t GetVertexStride() const override;
	public:
		using DebugPoint::DebugPoint;
	};
};
#endif
#endif