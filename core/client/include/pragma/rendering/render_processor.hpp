/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __RENDER_PROCESSOR_HPP__
#define __RENDER_PROCESSOR_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <mathutil/uvec.h>

class CMaterial;
class CBaseEntity;
class ModelSubMesh;
class CModelSubMesh;
struct RenderPassStats;
class RenderDebugInfo;
namespace util {
	struct RenderPassDrawInfo;
};
namespace pragma {
	class ShaderGameWorld;
	class CRenderComponent;
	class CVertexAnimatedComponent;
	class CModelComponent;
	class CLightMapReceiverComponent;
	class CRasterizationRendererComponent;
	class ShaderTextured3DBase;
	enum class ShaderGameWorldPipeline : uint32_t;
};
using MaterialIndex = uint32_t;
using EntityIndex = uint32_t;
enum class RenderFlags : uint32_t;
namespace prosper {
	class Shader;
};
namespace pragma::rendering {
	extern DLLCLIENT bool VERBOSE_RENDER_OUTPUT_ENABLED;
	enum class PassType : uint32_t {
		Generic = 0u,
		Reflection,

		Count
	};

	class DLLCLIENT ShaderProcessor {
	  public:
		ShaderProcessor(prosper::ICommandBuffer &cmdBuffer, PassType passType) : m_cmdBuffer {cmdBuffer}, m_passType {passType} {}
		bool RecordBindShader(const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, bool view, ShaderGameWorld::SceneFlags sceneFlags, pragma::ShaderGameWorld &shader, uint32_t pipelineIdx = 0u);
		bool RecordBindEntity(CBaseEntity &ent);
		bool RecordBindMaterial(CMaterial &mat);
		bool RecordBindLight(CLightComponent &light, uint32_t layerId);
		bool RecordDraw(CModelSubMesh &mesh, pragma::RenderMeshIndex meshIdx, const pragma::rendering::RenderQueue::InstanceSet *instanceSet = nullptr);

		void SetStats(RenderPassStats *stats) { m_stats = stats; }
		void SetDrawOrigin(const Vector4 &drawOrigin);
		void SetClipPlane(const std::optional<Vector4> &clipPlane) { m_clipPlane = clipPlane; }

		inline prosper::ICommandBuffer &GetCommandBuffer() const { return m_cmdBuffer; }
        inline prosper::IShaderPipelineLayout &GetCurrentPipelineLayout() const { return *m_currentPipelineLayout; }
        CBaseEntity &GetCurrentEntity() const;
        const pragma::CSceneComponent &GetCurrentScene() const;
		PassType GetPassType() const { return m_passType; }
	  private:
		bool RecordBindScene(const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, const pragma::ShaderGameWorld &referenceShader, bool view);
		bool BindInstanceSet(pragma::ShaderGameWorld &shaderScene, const pragma::rendering::RenderQueue::InstanceSet *instanceSet = nullptr);
		void UpdateSceneFlags(ShaderGameWorld::SceneFlags sceneFlags);
		void UpdateClipPlane();

		prosper::ICommandBuffer &m_cmdBuffer;
		std::unique_ptr<prosper::IShaderPipelineLayout> m_currentPipelineLayout = nullptr;
		std::optional<Vector4> m_clipPlane {};
		std::optional<Vector4> m_entityClipPlane {};
		Vector4 m_boundClipPlane {};
		std::optional<Vector2> m_depthBias {};
		Vector4 m_drawOrigin {};
		const pragma::CSceneComponent *m_sceneC = nullptr;
		const CRasterizationRendererComponent *m_rendererC = nullptr;
		pragma::CVertexAnimatedComponent *m_vertexAnimC = nullptr;
		pragma::CModelComponent *m_modelC = nullptr;
		pragma::CLightMapReceiverComponent *m_lightMapReceiverC = nullptr;
		ShaderGameWorld::SceneFlags m_sceneFlags = ShaderGameWorld::SceneFlags::None;

		PassType m_passType = PassType::Generic;
		uint32_t m_materialDescriptorSetIndex = std::numeric_limits<uint32_t>::max();
		uint32_t m_entityInstanceDescriptorSetIndex = std::numeric_limits<uint32_t>::max();
		const RenderQueue::InstanceSet *m_curInstanceSet = nullptr;

		pragma::ShaderGameWorld *m_curShader = nullptr;
		uint32_t m_curVertexAnimationOffset = std::numeric_limits<uint32_t>::max();
		RenderPassStats *m_stats = nullptr;

		float m_alphaCutoff = std::numeric_limits<float>::max();
		bool m_depthPrepass = false;
	};

	class DLLCLIENT BaseRenderProcessor {
	  public:
		enum class StateFlags : uint8_t {
			None = 0u,
			ShaderBound = 1u,
			MaterialBound = ShaderBound << 1u,
			EntityBound = MaterialBound << 1u,

			CountNonOpaqueMaterialsOnly = EntityBound << 1u
		};
		enum class CameraType : uint8_t { World = 0, View };
		BaseRenderProcessor(const util::RenderPassDrawInfo &drawSceneInfo, const Vector4 &drawOrigin);
		~BaseRenderProcessor();
		void SetCameraType(CameraType camType);
		void Set3DSky(bool enabled);
		void SetDrawOrigin(const Vector4 &drawOrigin);
		bool BindShader(prosper::PipelineID pipelineId);
		virtual bool BindShader(prosper::Shader &shader, uint32_t pipelineIdx = 0u);
		bool BindMaterial(CMaterial &mat);
		virtual bool BindEntity(CBaseEntity &ent);
		void SetDepthBias(float d, float delta);
		bool Render(CModelSubMesh &mesh, pragma::RenderMeshIndex meshIdx, const RenderQueue::InstanceSet *instanceSet = nullptr);
		pragma::ShaderGameWorld *GetCurrentShader();
		void UnbindShader();
		void SetCountNonOpaqueMaterialsOnly(bool b);
		prosper::Extent2D GetExtents() const;
		void RecordViewport();
		const util::RenderPassDrawInfo &GetRenderPassDrawInfo() const { return m_drawSceneInfo; }
	  protected:
		uint32_t Render(const pragma::rendering::RenderQueue &renderQueue, bool bindShaders, RenderPassStats *optStats = nullptr, std::optional<uint32_t> worldRenderQueueIndex = {});
		bool BindInstanceSet(pragma::ShaderGameWorld &shaderScene, const RenderQueue::InstanceSet *instanceSet = nullptr);
		void UnbindMaterial();
		void UnbindEntity();
		MaterialIndex m_curMaterialIndex = std::numeric_limits<MaterialIndex>::max();
		EntityIndex m_curEntityIndex = std::numeric_limits<EntityIndex>::max();

		ShaderProcessor m_shaderProcessor;

		prosper::Shader *m_curShader = nullptr;
		prosper::PipelineID m_curPipeline = std::numeric_limits<prosper::PipelineID>::max();
		pragma::ShaderGameWorld *m_shaderScene = nullptr;
		CMaterial *m_curMaterial = nullptr;
		CBaseEntity *m_curEntity = nullptr;
		pragma::CRenderComponent *m_curRenderC = nullptr;
		std::vector<std::shared_ptr<ModelSubMesh>> *m_curEntityMeshList = nullptr;
		const RenderQueue::InstanceSet *m_curInstanceSet = nullptr;
		ShaderGameWorld::SceneFlags m_baseSceneFlags = ShaderGameWorld::SceneFlags::None;

		uint32_t TranslateBasePipelineIndexToPassPipelineIndex(prosper::Shader &shader, uint32_t pipelineIdx, PassType passType) const;

		CameraType m_camType = CameraType::World;
		const util::RenderPassDrawInfo &m_drawSceneInfo;
		Vector4 m_drawOrigin;
		std::optional<Vector2> m_depthBias {};
		RenderPassStats *m_stats = nullptr;
		const pragma::CRasterizationRendererComponent *m_renderer = nullptr;
		uint32_t m_numShaderInvocations = 0;
		StateFlags m_stateFlags = StateFlags::None;
	};

	class DLLCLIENT DepthStageRenderProcessor : public pragma::rendering::BaseRenderProcessor {
	  public:
		DepthStageRenderProcessor(const util::RenderPassDrawInfo &drawSceneInfo, const Vector4 &drawOrigin);
		uint32_t Render(const pragma::rendering::RenderQueue &renderQueue, RenderPassStats *optStats = nullptr, std::optional<uint32_t> worldRenderQueueIndex = {});
		void BindLight(CLightComponent &light, uint32_t layerId);
	};

	class DLLCLIENT LightingStageRenderProcessor : public pragma::rendering::BaseRenderProcessor {
	  public:
		using pragma::rendering::BaseRenderProcessor::BaseRenderProcessor;
		uint32_t Render(const pragma::rendering::RenderQueue &renderQueue, RenderPassStats *optStats = nullptr, std::optional<uint32_t> worldRenderQueueIndex = {});
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::BaseRenderProcessor::StateFlags);

#endif
