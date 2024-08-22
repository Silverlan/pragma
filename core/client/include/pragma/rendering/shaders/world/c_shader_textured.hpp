/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_TEXTURED_HPP__
#define __C_SHADER_TEXTURED_HPP__

#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <sharedutils/alpha_mode.hpp>

namespace prosper {
	class IDescriptorSet;
};
class Texture;
namespace pragma {
	const float DefaultParallaxHeightScale = 0.025f;
	const uint16_t DefaultParallaxSteps = 16;
	const float DefaultAlphaDiscardThreshold = 0.99f;

	namespace rendering {
		enum class PassType : uint32_t;
	};

	enum class GameShaderSpecializationConstantFlag : uint32_t {
		None = 0u,

		// Static
		EnableLightMapsBit = 1u,
		EnableAnimationBit = EnableLightMapsBit << 1u,
		EnableMorphTargetAnimationBit = EnableAnimationBit << 1u,

		EnableTranslucencyBit = EnableMorphTargetAnimationBit << 1u,

		PermutationCount = (EnableTranslucencyBit << 1u) - 1,
		Last = EnableTranslucencyBit
	};

	class DLLCLIENT ShaderSpecializationManager {
	  public:
		using PassTypeIndex = uint32_t;
		using SpecializationFlags = uint64_t;
		using ConstantId = uint32_t;
		uint32_t GetPipelineCount() const { return m_pipelineSpecializations.size(); }
		PassTypeIndex GetBasePassType(uint32_t pipelineIdx) const;
		uint32_t GetPipelineIndexStartOffset(PassTypeIndex passType) const;
		std::optional<uint32_t> FindSpecializationPipelineIndex(PassTypeIndex passType, SpecializationFlags specializationFlags) const;
		template<typename TPassType, typename TSpecializationFlags>
		std::optional<uint32_t> FindSpecializationPipelineIndex(TPassType passType, TSpecializationFlags specializationFlags) const
		{
			return FindSpecializationPipelineIndex(static_cast<PassTypeIndex>(passType), static_cast<SpecializationFlags>(specializationFlags));
		}
	  protected:
		bool IsSpecializationConstantSet(uint32_t pipelineIdx, SpecializationFlags flag) const;
		void SetPipelineIndexRange(PassTypeIndex passType, uint32_t startIndex, uint32_t endIndex);
		template<typename TSpecializationConstantFlag>
		bool IsSpecializationConstantSet(uint32_t pipelineIdx, TSpecializationConstantFlag flag) const
		{
			return IsSpecializationConstantSet(pipelineIdx, static_cast<SpecializationFlags>(flag));
		}
		void RegisterSpecializations(PassTypeIndex passType, SpecializationFlags staticFlags, SpecializationFlags dynamicFlags);
		template<typename TPassType, typename TSpecializationConstantFlag>
		void RegisterSpecializations(TPassType passType, TSpecializationConstantFlag staticFlags, TSpecializationConstantFlag dynamicFlags)
		{
			RegisterSpecializations(static_cast<PassTypeIndex>(passType), static_cast<SpecializationFlags>(staticFlags), static_cast<SpecializationFlags>(dynamicFlags));
		}
		template<typename TSpecializationConstantFlag>
		bool AddSpecializationConstant(prosper::ShaderGraphics &shader, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx, prosper::ShaderStageFlags stageFlags, TSpecializationConstantFlag flag)
		{
			uint32_t isSet = static_cast<uint32_t>(IsSpecializationConstantSet(pipelineIdx, flag));
			return shader.AddSpecializationConstant(pipelineInfo, stageFlags, umath::get_least_significant_set_bit_index(umath::to_integral(flag)), sizeof(uint32_t), &isSet);
		}
	  private:
		std::vector<SpecializationFlags> m_pipelineSpecializations;
		struct PassTypeInfo {
			std::array<uint32_t, umath::to_integral(GameShaderSpecializationConstantFlag::PermutationCount)> specializationToPipelineIdx;
			std::pair<uint32_t, uint32_t> pipelineIndexRange {0, 0};
		};
		PassTypeInfo &InitializePassTypeSpecializations(PassTypeIndex passType);
		// Per pass-type
		std::vector<PassTypeInfo> m_passTypeSpecializationToPipelineIdx;
	};

	enum class GameShaderSpecializationPropertyIndex : uint32_t {
		Start = umath::get_least_significant_set_bit_index_c(umath::to_integral(GameShaderSpecializationConstantFlag::Last)) + 1,
		ShadowQuality = Start,
		DebugModeEnabled,
		BloomOutputEnabled,
		EnableSsao,
		EnableIbl,
		EnableDynamicLighting,
		EnableDynamicShadows
	};
	enum class GameShaderSpecialization : uint32_t { Generic = 0, Lightmapped, Animated, Count };

	class DLLCLIENT ShaderGameWorldLightingPass : public ShaderGameWorld, public ShaderSpecializationManager {
	  public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_RENDER_BUFFER_INDEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX;

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

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDERER;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SHADOWS;

		enum class VertexBinding : uint32_t { LightmapUv = umath::to_integral(ShaderEntity::VertexBinding::Count) };

		enum class MaterialBinding : uint32_t {
			MaterialSettings = 0u,
			DiffuseMap,
			NormalMap,
			SpecularMap,
			ParallaxMap,
			GlowMap,

			Count
		};

		enum class InstanceBinding : uint32_t { Instance = 0u, BoneMatrices, VertexAnimations, VertexAnimationFrameData };

		enum class MaterialFlags : uint32_t {
			None = 0,
			Diffuse = 0,
			Normal = 1,
			Parallax = Normal << 1,
			Glow = Parallax << 1,
			Translucent = Glow << 1,
			BlackToAlpha = Translucent << 1,

			FMAT_GLOW_MODE_1 = BlackToAlpha << 1,
			FMAT_GLOW_MODE_2 = FMAT_GLOW_MODE_1 << 1,
			FMAT_GLOW_MODE_3 = FMAT_GLOW_MODE_2 << 1,
			FMAT_GLOW_MODE_4 = FMAT_GLOW_MODE_3 << 1,

			DiffuseSRGB = FMAT_GLOW_MODE_4 << 1u,
			GlowSRGB = DiffuseSRGB << 1u,
			Debug = GlowSRGB << 1u,

			WrinkleMaps = Debug << 1u,
			RmaMap = WrinkleMaps << 1u
		};

#pragma pack(push, 1)
		struct PushConstants : public ScenePushConstants {
			static_assert(sizeof(pragma::SceneDebugMode) == sizeof(uint32_t));
			pragma::SceneDebugMode debugMode;
			float reflectionProbeIntensity;
			Vector2 padding;

			void Initialize()
			{
				ScenePushConstants::Initialize();
				debugMode = pragma::SceneDebugMode::None;
				reflectionProbeIntensity = 1.f;
			}
		};

		struct MaterialData {
			Vector4 color = {1.f, 1.f, 1.f, 1.f};
			Vector4 emissionFactor = {1.f, 1.f, 1.f, 1.f};
			MaterialFlags flags = MaterialFlags::Diffuse;
			float glowScale = 1.f;
			int16_t parallaxHeightScale = glm::detail::toFloat16(DefaultParallaxHeightScale);
			uint16_t parallaxSteps = DefaultParallaxSteps;
			float alphaDiscardThreshold = DefaultAlphaDiscardThreshold;
			float phongIntensity = 1.f;
			float metalnessFactor = 0.f;
			float roughnessFactor = 0.f;
			float aoFactor = 1.f;
			AlphaMode alphaMode = AlphaMode::Opaque;
			float alphaCutoff = 0.5f;

			void SetParallaxHeightScale(float scale) { parallaxHeightScale = glm::detail::toFloat16(DefaultParallaxHeightScale); }
			float GetParallaxHeightScale() const { return glm::detail::toFloat32(DefaultParallaxHeightScale); }
		};
#pragma pack(pop)

		ShaderGameWorldLightingPass(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");
		virtual ~ShaderGameWorldLightingPass() override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual bool GetRenderBufferTargets(CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const override;
		static MaterialData GenerateMaterialData(CMaterial &mat);
		std::optional<MaterialData> UpdateMaterialBuffer(CMaterial &mat);
		bool RecordPushSceneConstants(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const Vector4 &drawOrigin) const;

		virtual void InitializeMaterialData(CMaterial &mat, MaterialData &matData);
		virtual uint32_t GetMaterialDescriptorSetIndex() const override;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetRendererDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override;
		virtual uint32_t GetLightDescriptorSetIndex() const override;
		virtual uint32_t GetPassPipelineIndexStartOffset(rendering::PassType passType) const override;
		std::optional<uint32_t> FindPipelineIndex(rendering::PassType passType, GameShaderSpecialization specialization, GameShaderSpecializationConstantFlag specializationFlags) const;
		virtual GameShaderSpecializationConstantFlag GetMaterialPipelineSpecializationRequirements(CMaterial &mat) const;

		//
		virtual GameShaderSpecializationConstantFlag GetBaseSpecializationFlags() const;
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsLights, prosper::IDescriptorSet &dsShadows, prosper::IDescriptorSet &dsMaterial, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const override;
		virtual bool IsUsingLightmaps() const override { return true; }
		bool IsDepthPrepassEnabled() const;
		void SetDepthPrepassEnabled(bool enabled) { m_depthPrepassEnabled = enabled; }
		static std::optional<MaterialData> InitializeMaterialBuffer(prosper::IDescriptorSet &descSet, CMaterial &mat, uint32_t bindingIdx);
		static std::shared_ptr<Texture> GetTexture(const std::string &texName);
	  protected:
		using ShaderEntity::RecordDraw;
		GameShaderSpecializationConstantFlag GetStaticSpecializationConstantFlags(GameShaderSpecialization specialization) const;
		virtual void OnPipelinesInitialized() override;
		virtual void ApplyMaterialFlags(CMaterial &mat, MaterialFlags &outFlags) const;
		virtual void UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags);
		std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo);
		std::optional<MaterialData> InitializeMaterialBuffer(prosper::IDescriptorSet &descSet, CMaterial &mat);
		virtual void InitializeGfxPipelineVertexAttributes(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx);
		virtual void InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx);
		virtual void InitializeGfxPipelineDescriptorSets(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx);
		virtual prosper::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) override;
		bool m_depthPrepassEnabled = true;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderGameWorldLightingPass::MaterialFlags)
REGISTER_BASIC_BITWISE_OPERATORS(pragma::GameShaderSpecializationConstantFlag)

#endif
