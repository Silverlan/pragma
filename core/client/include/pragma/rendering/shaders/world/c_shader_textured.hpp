#ifndef __C_SHADER_TEXTURED_HPP__
#define __C_SHADER_TEXTURED_HPP__

#include "pragma/rendering/shaders/world/c_shader_scene.hpp"

namespace pragma
{
	const float DefaultParallaxHeightScale = 0.025f;
	const float DefaultAlphaDiscardThreshold = 0.99f;
	class DLLCLIENT ShaderTextured3DBase
		: public ShaderEntity
	{
	public:
		enum class Pipeline : uint32_t
		{
			Regular = umath::to_integral(ShaderScene::Pipeline::Regular),
			MultiSample = umath::to_integral(ShaderScene::Pipeline::MultiSample),
			//LightMap = umath::to_integral(ShaderScene::Pipeline::LightMap),

			Reflection = umath::to_integral(ShaderScene::Pipeline::Count),
			Count
		};
		static Pipeline GetPipelineIndex(Anvil::SampleCountFlagBits sampleCount,bool bReflection=false);
		static uint32_t HASH_TYPE;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_ID;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_UV;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_NORMAL;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_TANGENT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BI_TANGENT;
		
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_LIGHTMAP;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_LIGHTMAP_UV;

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CSM;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_SHADOWS;

		enum class MaterialBinding : uint32_t
		{
			DiffuseMap = 0u,
			NormalMap,
			SpecularMap,
			ParallaxMap,
			GlowMap,

			Count
		};

		enum class InstanceBinding : uint32_t
		{
			Instance = 0u,
			BoneMatrices,
			VertexAnimations,
			VertexAnimationFrameData
		};

		enum class MaterialFlags : uint32_t
		{
			None = 0,
			Diffuse = 0,
			Normal = 1,
			Specular = Normal<<1,
			SpecularMapDefined = Specular<<1,
			SpecularMap = SpecularMapDefined | Specular,
			Parallax = Specular<<2,
			Glow = Parallax<<1,
			Translucent = Glow<<1,
			DiffuseSpecular = Translucent<<1, // Specular from diffuse alpha
			NormalSpecular = DiffuseSpecular<<1, // Specular from normal alpha
			BlackToAlpha = NormalSpecular<<1,

			FMAT_GLOW_MODE_1 = BlackToAlpha<<1,
			FMAT_GLOW_MODE_2 = FMAT_GLOW_MODE_1<<1,
			FMAT_GLOW_MODE_3 = FMAT_GLOW_MODE_2<<1,

			NoIBL = FMAT_GLOW_MODE_3<<1u // PBR only (see PBR shader)
		};

		enum class StateFlags : uint32_t
		{
			None = 0u,
			ClipPlaneBound = 1u,
			ShouldUseLightMap = ClipPlaneBound<<1u
		};

#pragma pack(push,1)
		struct PushConstants
		{
			struct Material
			{
				Vector4 phong;
				MaterialFlags flags;
				uint32_t lightmapFlags;
				float glowScale;
				float parallaxHeightScale;
				float alphaDiscardThreshold;
				float phongIntensity;
			};
			Vector3 clipPlane;
			uint32_t vertexAnimInfo;
			Material material;
		};
#pragma pack(pop)

		ShaderTextured3DBase(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");

		virtual bool BindClipPlane(const Vector4 &clipPlane);
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,Pipeline pipelineIdx=Pipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		);
		virtual size_t GetBaseTypeHashCode() const override;
		virtual std::shared_ptr<prosper::DescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool BindMaterial(CMaterial &mat);
		virtual bool Draw(CModelSubMesh &mesh) override;
	protected:
		using ShaderEntity::Draw;
		bool BindLightMapUvBuffer(CModelSubMesh &mesh);
		virtual void ApplyMaterialFlags(CMaterial &mat,MaterialFlags &outFlags) const;
		virtual void OnPipelineBound() override;
		virtual void OnPipelineUnbound() override;
		virtual bool BindMaterialParameters(CMaterial &mat);
		std::shared_ptr<prosper::DescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::Shader::DescriptorSetInfo &descSetInfo);
		virtual void InitializeGfxPipelineVertexAttributes(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		virtual void InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		virtual void InitializeGfxPipelineDescriptorSets(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		virtual prosper::Shader::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const;
		virtual uint32_t GetMaterialDescriptorSetIndex() const;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual uint32_t GetLightDescriptorSetIndex() const override;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		StateFlags m_stateFlags = StateFlags::ShouldUseLightMap;
	};

	////////////////////////////

	class DLLCLIENT ShaderTextured3D
		: public ShaderTextured3DBase
	{
	public:
		ShaderTextured3D(prosper::Context &context,const std::string &identifier);
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderTextured3DBase::MaterialFlags)
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderTextured3DBase::StateFlags)

#endif
