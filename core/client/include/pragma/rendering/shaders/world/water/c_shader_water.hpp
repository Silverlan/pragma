#ifndef __C_SHADER_WATER_HPP__
#define __C_SHADER_WATER_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderWater
		: public ShaderTextured3DBase
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_WATER;
		enum class MaterialBinding : uint32_t
		{
			DuDvMap = 0u,
			NormalMap,

			Count
		};

		enum class WaterBinding : uint32_t
		{
			ReflectionMap = 0u,
			RefractionMap,
			RefractionDepth,
			WaterSettings,
			WaterFog,

			Count
		};

#pragma pack(push,1)
		struct PushConstants
		{
			float waterFogIntensity;
			uint32_t enableReflection;
			Mat4 reflectionVp;
		};
#pragma pack(pop)

		ShaderWater(prosper::Context &context,const std::string &identifier);
		void SetReflectionEnabled(bool b);
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f},Pipeline pipelineIdx=Pipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		virtual void EndDraw() override;
		virtual bool BindSceneCamera(const rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BindEntity(CBaseEntity &ent) override;
		virtual std::shared_ptr<prosper::DescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelineDescriptorSets(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual prosper::Shader::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const;
	private:
		bool UpdateBindFogDensity();
		bool m_bReflectionEnabled = false;
		std::weak_ptr<Scene> m_boundScene = {};
		EntityHandle m_boundEntity = {};
	};
};

// prosper TODO
#if 0
//#include "pragma/rendering/shaders/world/c_shader_reflective.h"
#include "pragma/rendering/shaders/world/c_shader_textured.h"

namespace Shader
{
	class DLLCLIENT Water
		: public Textured3D
	{
	public:
		enum class DLLCLIENT DescSet : uint32_t
		{
			WaterEffects = umath::to_integral(Textured3D::DescSet::ShadowCubeMaps) +1,
			WaterSettings = WaterEffects,
			WaterFog = WaterSettings
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			DudvMap = 0,
			NormalMap = DudvMap +1,

			ReflectionMap = 0,
			RefractionMap = ReflectionMap +1,
			RefractionDepthMap = RefractionMap +1,

			WaterSettings = RefractionDepthMap +1,
			WaterFog = WaterSettings +1
		};

		Water();
		using Textured3D::BeginDraw;
		using Textured3D::BindEntity;
		virtual void InitializeMaterial(Material *mat,bool bReload=false) override;
		void SetReflectionEnabled(bool b);
	protected:
		bool m_bReflectionEnabled = true;
		virtual void InitializeMaterialBindings(std::vector<Vulkan::DescriptorSetLayout::Binding> &bindings) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline) override;
		virtual bool BindEntity(Vulkan::CommandBufferObject *cmdBuffer,CBaseEntity *ent,bool &bWeighted) override;
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
	};
};
#endif
#endif