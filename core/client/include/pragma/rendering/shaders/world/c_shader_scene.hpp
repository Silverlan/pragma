/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_SCENE_HPP__
#define __C_SHADER_SCENE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/c_shader_3d.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured_base.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include <shader/prosper_shader.hpp>

class CModelSubMesh;
namespace prosper {
	class IRenderBuffer;
};
namespace pragma {
	using RenderMeshIndex = uint32_t;
	class CSceneComponent;
	class SceneMesh;
	enum class SceneDebugMode : uint32_t;
	class CRasterizationRendererComponent;
	namespace rendering {
		enum class PassType : uint32_t;
	};
	class DLLCLIENT ShaderScene : public Shader3DBase, public ShaderTexturedBase {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SCENE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDERER;

		static prosper::Format RENDER_PASS_FORMAT;
		static prosper::Format RENDER_PASS_DEPTH_FORMAT;

		static prosper::SampleCountFlags RENDER_PASS_SAMPLES;
		static void SetRenderPassSampleCount(prosper::SampleCountFlags samples);

		enum class SceneBinding : uint32_t {
			Camera = 0u,
			RenderSettings,
		};

		enum class RendererBinding : uint32_t {
			Renderer = 0u,
			SSAOMap,

			LightBuffers,
			TileVisLightIndexBuffer,
			ShadowData,
			CSM,

			LightMapDiffuse,
			LightMapDiffuseIndirect,
			LightMapDominantDirection,
		};

		enum class RenderSettingsBinding : uint32_t {
			Debug = 0u,
			Time,
			CSMData,
			GlobalInstance,
			// #ifdef PRAGMA_ENABLE_SHADER_DEBUG_PRINT
			DebugPrint,
			// #endif
		};

		enum class DebugFlags : uint32_t {
			None = 0u,
			LightShowCascades = 1u,
			LightShowShadowMapDepth = LightShowCascades << 1u,
			LightShowFragmentDepthShadowSpace = LightShowShadowMapDepth << 1u,

			ForwardPlusHeatmap = LightShowFragmentDepthShadowSpace << 1u,
			Unlit = ForwardPlusHeatmap << 1u
		};

#pragma pack(push, 1)
		struct TimeData {
			float curTime;
			float deltaTime;
			float realTime;
			float deltaRealTime;
		};
		struct DebugData {
			uint32_t flags;
		};
#pragma pack(pop)
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const = 0;
		virtual uint32_t GetCameraDescriptorSetIndex() const = 0;
		virtual uint32_t GetRendererDescriptorSetIndex() const { return std::numeric_limits<uint32_t>::max(); }
		virtual std::optional<std::string> GetGlslPrefixCode(prosper::ShaderStage stage) const override;
		virtual bool IsDebugPrintEnabled() const;
	  protected:
		ShaderScene(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");
		prosper::SampleCountFlags GetSampleCount(uint32_t pipelineIdx) const;
		virtual bool ShouldInitializePipeline(uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass, uint32_t pipelineIdx) override;
	};

	/////////////////////

	class DLLCLIENT ShaderSceneLit : public ShaderScene {
	  public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SHADOWS;

		enum class LightBinding : uint32_t {
			LightBuffers = 0u,
			TileVisLightIndexBuffer,
			ShadowData,
			CSM,
		};

		enum class ShadowBinding : uint32_t {
			ShadowMaps = 0u,
			ShadowCubeMaps,
		};

#pragma pack(push, 1)
		struct CSMData {
			std::array<Mat4, 4> VP;
			Vector4 fard;
			int32_t count;
		};
#pragma pack(pop)
	  protected:
		ShaderSceneLit(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");
	};

	/////////////////////

	class DLLCLIENT ShaderEntity : public ShaderSceneLit {
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

		enum class VertexBinding : uint32_t {
			RenderBufferIndex = 0u,

			BoneWeight,
			BoneWeightExt,

			Vertex,

			Count
		};

#pragma pack(push, 1)
		struct InstanceData {
			enum class RenderFlags : uint32_t { None = 0u, Weighted = 1u };
			Mat4 modelMatrix;
			Vector4 color;
			RenderFlags renderFlags;
			uint32_t entityIndex;
			Vector2 padding;
		};
#pragma pack(pop)

		virtual bool GetRenderBufferTargets(CModelSubMesh &mesh, uint32_t pipelineIdx, std::vector<prosper::IBuffer *> &outBuffers, std::vector<prosper::DeviceSize> &outOffsets, std::optional<prosper::IndexBufferInfo> &outIndexBufferInfo) const;
		virtual uint32_t GetInstanceDescriptorSetIndex() const = 0;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const = 0;
		std::shared_ptr<prosper::IRenderBuffer> CreateRenderBuffer(CModelSubMesh &mesh, uint32_t pipelineIdx) const;
	  protected:
		ShaderEntity(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader = "");
	};

	namespace rendering {
		class ShaderProcessor;
	};
	class DLLCLIENT ShaderGameWorld : public ShaderEntity {
	  public:
		enum class SceneFlags : uint32_t {
			None = 0u,
			UseExtendedVertexWeights = 1u,
			RenderAs3DSky = UseExtendedVertexWeights << 1u,
			AlphaTest = RenderAs3DSky << 1u,
			LightmapsEnabled = AlphaTest << 1u,
			NoIBL = LightmapsEnabled << 1u,
			DisableShadows = NoIBL << 1u,
			IndirectLightmapsEnabled = DisableShadows << 1u,
			DirectionalLightmapsEnabled = IndirectLightmapsEnabled << 1u
		};
		enum class GameShaderType : uint8_t { LightingPass = 0, DepthPrepass, ShadowPass, SkyPass };
#pragma pack(push, 1)
		struct ScenePushConstants {
			Vector4 clipPlane;
			Vector4 drawOrigin; // w is scale
			Vector2 depthBias;
			uint32_t vertexAnimInfo;
			SceneFlags flags;

			void Initialize()
			{
				clipPlane = {};
				drawOrigin = {0.f, 0.f, 0.f, 1.f};
				depthBias = {};
				vertexAnimInfo = 0;
				flags = SceneFlags::None;
			}
		};
#pragma pack(pop)

		static uint32_t HASH_TYPE;
		using ShaderEntity::ShaderEntity;

		virtual bool IsDepthPrepassShader() const { return false; }
		virtual GameShaderType GetPassType() const { return GameShaderType::LightingPass; }
		virtual size_t GetBaseTypeHashCode() const override;
		std::optional<uint32_t> GetMaterialDescriptorSetIndex() const;
		virtual uint32_t GetSceneDescriptorSetIndex() const { return std::numeric_limits<uint32_t>::max(); };
		const prosper::DescriptorSetInfo *GetMaterialDescriptorSetInfo() const;

		// Note: All recording functions are called in a multi-threaded environment! Handle with care!
		virtual void RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
		  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const
		{
		}
		virtual bool RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const;
		virtual bool RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, CMaterial &mat) const;
		virtual void RecordSceneFlags(rendering::ShaderProcessor &shaderProcessor, SceneFlags sceneFlags) const;
		virtual void RecordBindLight(rendering::ShaderProcessor &shaderProcessor, CLightComponent &light, uint32_t layerId) const {}
		virtual void RecordAlphaCutoff(rendering::ShaderProcessor &shaderProcessor, float alphaCutoff) const {}
		virtual void RecordClipPlane(rendering::ShaderProcessor &shaderProcessor, const Vector4 &clipPlane) const;
		virtual void RecordDrawOrigin(rendering::ShaderProcessor &shaderProcessor, const Vector4 &drawOrigin) const;
		virtual void RecordDepthBias(rendering::ShaderProcessor &shaderProcessor, const Vector2 &depthBias) const;
		virtual void RecordVertexAnimationOffset(rendering::ShaderProcessor &shaderProcessor, uint32_t vertexAnimationOffset) const;
		virtual bool OnRecordDrawMesh(rendering::ShaderProcessor &shaderProcessor, CModelSubMesh &mesh) const { return true; }
		virtual bool IsUsingLightmaps() const { return false; }
		virtual uint32_t GetPassPipelineIndexStartOffset(rendering::PassType passType) const { return 0; }
	  protected:
		SceneFlags m_sceneFlags = SceneFlags::None;
		std::unique_ptr<prosper::DescriptorSetInfo> m_materialDescSetInfo;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderEntity::InstanceData::RenderFlags);
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderScene::DebugFlags);
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderGameWorld::SceneFlags);

#endif
