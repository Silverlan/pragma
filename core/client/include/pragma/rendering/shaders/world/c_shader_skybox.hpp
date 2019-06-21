#ifndef __C_SHADER_SKYBOX_H__
#define __C_SHADER_SKYBOX_H__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderSkybox
		: public ShaderTextured3DBase
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;

#pragma pack(push,1)
		struct PushConstants
		{
			Vector3 posCam;
		};
#pragma pack(pop)

		ShaderSkybox(prosper::Context &context,const std::string &identifier);
		virtual std::shared_ptr<prosper::DescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,Pipeline pipelineIdx=Pipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		virtual bool BindRenderSettings(Anvil::DescriptorSet &descSetRenderSettings) override;
		virtual bool BindSceneCamera(const pragma::rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BindLights(Anvil::DescriptorSet &descSetShadowMaps,Anvil::DescriptorSet &descSetLightSources) override;
		virtual bool BindVertexAnimationOffset(uint32_t offset) override;
		virtual bool Draw(CModelSubMesh &mesh) override;
	protected:
		virtual uint32_t GetMaterialDescriptorSetIndex() const override;
		virtual bool BindMaterialParameters(CMaterial &mat) override;
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"

namespace Shader
{
	class DLLCLIENT Skybox
		: public Textured3D
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			Instance = 0,
			Camera = Instance +1,
			SkyboxMap = Camera +1
		};
		enum class DLLCLIENT Location : uint32_t
		{
			Vertex = 0
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Vertex = 0,

			Instance = 0,
			Camera = 0,
			SkyboxMap = 0
		};
	protected:
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
		virtual bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline) override;
	public:
		Skybox();
		virtual void InitializeMaterial(Material *mat,bool bReload=false) override;
		virtual void BindLights(const Vulkan::DescriptorSetObject *descSetSpot,const Vulkan::DescriptorSetObject *descSetPoint) override;
		virtual void BindScene(Vulkan::CommandBufferObject *cmdBuffer,const Scene &scene,bool bView=false) override;
		virtual void Draw(CModelSubMesh *mesh) override;
		virtual void BindMaterial(Material *mat) override;
		using Textured3D::BeginDraw;
	};
};
#endif
#endif