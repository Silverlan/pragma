/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_TEXTURED_HPP__
#define __C_SHADER_TEXTURED_HPP__

#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <sharedutils/alpha_mode.hpp>

namespace prosper {class IDescriptorSet;};
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
		static Pipeline GetPipelineIndex(prosper::SampleCountFlags sampleCount,bool bReflection=false);
		static uint32_t HASH_TYPE;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_ID;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT_EXT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_UV;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_NORMAL;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_TANGENT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BI_TANGENT;
		
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_LIGHTMAP;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_LIGHTMAP_UV;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDERER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;

		enum class VertexBinding : uint32_t
		{
			LightmapUv = umath::to_integral(ShaderEntity::VertexBinding::Count)
		};

		enum class MaterialBinding : uint32_t
		{
			MaterialSettings = 0u,
			DiffuseMap,
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
			Parallax = Normal<<2,
			Glow = Parallax<<1,
			Translucent = Glow<<1,
			BlackToAlpha = Translucent<<1,

			FMAT_GLOW_MODE_1 = BlackToAlpha<<1,
			FMAT_GLOW_MODE_2 = FMAT_GLOW_MODE_1<<1,
			FMAT_GLOW_MODE_3 = FMAT_GLOW_MODE_2<<1,
			FMAT_GLOW_MODE_4 = FMAT_GLOW_MODE_3<<1,

			DiffuseSRGB = FMAT_GLOW_MODE_4<<1u,
			GlowSRGB = DiffuseSRGB<<1u
		};

		enum class StateFlags : uint32_t
		{
			None = 0u,
			ClipPlaneBound = 1u,
			ShouldUseLightMap = ClipPlaneBound<<1u,
			RenderAs3DSky = ShouldUseLightMap<<1u
		};

#pragma pack(push,1)
		enum class RenderFlags : uint32_t
		{
			None = 0u,
			LightmapsEnabled = 1u,

			// PBR only
			NoIBL = LightmapsEnabled<<1u,

			TranslucencyEnabled = NoIBL<<1u,
			UseExtendedVertexWeights = TranslucencyEnabled<<1u,
			Is3DSky = UseExtendedVertexWeights<<1u
		};

		struct PushConstants
		{
			Vector4 clipPlane; // w is reflection probe intensity
			Vector4 drawOrigin; // w is scale
			uint32_t vertexAnimInfo;
			RenderFlags flags;
			static_assert(sizeof(Scene::DebugMode) == sizeof(uint32_t));
			Scene::DebugMode debugMode;
			float padding; // Padding to vec4
		};

		struct MaterialData
		{
			Vector4 color = {1.f,1.f,1.f,1.f};
			Vector4 emissionFactor = {1.f,1.f,1.f,1.f};
			MaterialFlags flags = MaterialFlags::Diffuse;
			float glowScale = 1.f;
			float parallaxHeightScale = DefaultParallaxHeightScale;
			float alphaDiscardThreshold = DefaultAlphaDiscardThreshold;
			float phongIntensity = 1.f;
			float metalnessFactor = 0.f;
			float roughnessFactor = 0.f;
			float aoFactor = 1.f;
			AlphaMode alphaMode = AlphaMode::Opaque;
			float alphaCutoff = 0.5f;
		};
#pragma pack(pop)

		ShaderTextured3DBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		virtual ~ShaderTextured3DBase() override;
		virtual bool BindClipPlane(const Vector4 &clipPlane);
		virtual bool BeginDraw(
			const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f},Pipeline pipelineIdx=Pipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		);
		virtual size_t GetBaseTypeHashCode() const override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool BindMaterial(CMaterial &mat);
		virtual bool Draw(CModelSubMesh &mesh) override;
		bool BindReflectionProbeIntensity(float intensity);
		std::optional<MaterialData> UpdateMaterialBuffer(CMaterial &mat) const;
		bool SetDebugMode(Scene::DebugMode debugMode);
		void Set3DSky(bool is3dSky);
	protected:
		using ShaderEntity::Draw;
		bool BindLightMapUvBuffer(CModelSubMesh &mesh,bool &outShouldUseLightmaps);
		virtual void ApplyMaterialFlags(CMaterial &mat,MaterialFlags &outFlags) const;
		virtual void UpdateRenderFlags(CModelSubMesh &mesh,RenderFlags &inOutFlags);
		virtual void OnPipelineBound() override;
		virtual void OnPipelineUnbound() override;
		virtual bool BindMaterialParameters(CMaterial &mat);
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::DescriptorSetInfo &descSetInfo);
		std::optional<MaterialData> InitializeMaterialBuffer(prosper::IDescriptorSet &descSet,CMaterial &mat);
		virtual void InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		virtual void InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		virtual void InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx);
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const;
		virtual uint32_t GetMaterialDescriptorSetIndex() const;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetRendererDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual uint32_t GetLightDescriptorSetIndex() const override;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		StateFlags m_stateFlags = StateFlags::ShouldUseLightMap;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderTextured3DBase::MaterialFlags)
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderTextured3DBase::RenderFlags)
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderTextured3DBase::StateFlags)

#endif
