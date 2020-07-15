/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_SCENE_HPP__
#define __C_SHADER_SCENE_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/c_shader_3d.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured_base.hpp"
#include <shader/prosper_shader.hpp>

class CModelSubMesh;
namespace pragma
{
	namespace rendering {class RasterizationRenderer;};
	class DLLCLIENT ShaderScene
		: public Shader3DBase,
		public ShaderTexturedBase
	{
	public:
		enum class Pipeline : uint32_t
		{
			Regular = 0u,
			MultiSample,
			//LightMap,

			Count
		};
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDER_SETTINGS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_RENDERER;

		static prosper::Format RENDER_PASS_FORMAT;
		static prosper::Format RENDER_PASS_DEPTH_FORMAT;

		static prosper::SampleCountFlags RENDER_PASS_SAMPLES;
		static void SetRenderPassSampleCount(prosper::SampleCountFlags samples);

		enum class CameraBinding : uint32_t
		{
			Camera = 0u,
			RenderSettings
		};

		enum class RendererBinding : uint32_t
		{
			Renderer = 0u,
			SSAOMap,
			LightMap
		};

		enum class RenderSettingsBinding : uint32_t
		{
			Debug = 0u,
			Time,
			CSMData
		};

		enum class DebugFlags : uint32_t
		{
			None = 0u,
			LightShowCascades = 1u,
			LightShowShadowMapDepth = LightShowCascades<<1u,
			LightShowFragmentDepthShadowSpace = LightShowShadowMapDepth<<1u,

			ForwardPlusHeatmap = LightShowFragmentDepthShadowSpace<<1u
		};

#pragma pack(push,1)
		struct TimeData
		{
			float curTime;
			float deltaTime;
			float realTime;
			float deltaRealTime;
		};
		struct DebugData
		{
			uint32_t flags;
		};
#pragma pack(pop)

		virtual bool BindSceneCamera(const rendering::RasterizationRenderer &renderer,bool bView);
		virtual bool BindRenderSettings(prosper::IDescriptorSet &descSetRenderSettings);
	protected:
		ShaderScene(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		prosper::SampleCountFlags GetSampleCount(uint32_t pipelineIdx) const;
		virtual bool ShouldInitializePipeline(uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const=0;
		virtual uint32_t GetCameraDescriptorSetIndex() const=0;
		virtual uint32_t GetRendererDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
	};

	/////////////////////

	class DLLCLIENT ShaderSceneLit
		: public ShaderScene
	{
	public:
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_LIGHTS;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_SHADOWS;

		enum class LightBinding : uint32_t
		{
			LightBuffers = 0u,
			TileVisLightIndexBuffer,
			ShadowData,
			CSM
		};

		enum class ShadowBinding : uint32_t
		{
			ShadowMaps = 0u,
			ShadowCubeMaps,
		};

#pragma pack(push,1)
		struct CSMData
		{
			std::array<Mat4,4> VP;
			Vector4 fard;
			int32_t count;
		};
#pragma pack(pop)

		virtual bool BindLights(prosper::IDescriptorSet &dsLights);
		virtual bool BindScene(rendering::RasterizationRenderer &renderer,bool bView);
	protected:
		ShaderSceneLit(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		virtual uint32_t GetLightDescriptorSetIndex() const=0;
	};

	/////////////////////

	class DLLCLIENT ShaderEntity
		: public ShaderSceneLit
	{
	public:
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

		enum class VertexBinding : uint32_t
		{
			BoneWeight = 0u,
			BoneWeightExt,
			
			Vertex,

			Count
		};

#pragma pack(push,1)
		struct InstanceData
		{
			enum class RenderFlags : uint32_t
			{
				None = 0u,
				Weighted = 1u
			};
			Mat4 modelMatrix;
			Vector4 color;
			RenderFlags renderFlags;
			Vector3 padding;
		};
#pragma pack(pop)

		bool BindInstanceDescriptorSet(prosper::IDescriptorSet &descSet);
		virtual bool BindEntity(CBaseEntity &ent);
		virtual bool BindVertexAnimationOffset(uint32_t offset);
		virtual bool BindScene(rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool Draw(CModelSubMesh &mesh);
		virtual void EndDraw() override;
		CBaseEntity *GetBoundEntity();
	protected:
		ShaderEntity(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		bool Draw(CModelSubMesh &mesh,bool bUseVertexWeightBuffer);
		bool Draw(CModelSubMesh &mesh,const std::function<bool(CModelSubMesh&)> &fDraw,bool bUseVertexWeightBuffer);

		virtual uint32_t GetInstanceDescriptorSetIndex() const=0;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const=0;

		CBaseEntity *m_boundEntity = nullptr;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderEntity::InstanceData::RenderFlags);
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderScene::DebugFlags);

#endif
