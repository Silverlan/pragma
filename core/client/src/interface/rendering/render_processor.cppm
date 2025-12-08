// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.client:rendering.render_processor;

export import :entities.components.scene;

export namespace pragma {
	class CVertexAnimatedComponent;
};
export namespace pragma::rendering {
	enum class PassType : uint32_t {
		Generic = 0u,
		Reflection,

		Count
	};
	enum class RenderPass : uint8_t {
		Prepass = 0,
		Lighting,
		Shadow,
	};

	class DLLCLIENT ShaderProcessor {
	  public:
		ShaderProcessor(prosper::ICommandBuffer &cmdBuffer, PassType passType) : m_cmdBuffer {cmdBuffer}, m_passType {passType} {}
		bool RecordBindShader(const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, bool view, ShaderGameWorld::SceneFlags sceneFlags, pragma::ShaderGameWorld &shader, uint32_t pipelineIdx = 0u);
		bool RecordBindEntity(CBaseEntity &ent);
		bool RecordBindMaterial(msys::CMaterial &mat);
		bool RecordBindLight(CLightComponent &light, uint32_t layerId);
		bool RecordDraw(pragma::geometry::CModelSubMesh &mesh, pragma::rendering::RenderMeshIndex meshIdx, const pragma::rendering::RenderQueue::InstanceSet *instanceSet = nullptr);

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
		pragma::BaseModelComponent *m_modelC = nullptr;
		void *m_lightMapReceiverC = nullptr;
		ShaderGameWorld::SceneFlags m_sceneFlags = ShaderGameWorld::SceneFlags::None;

		PassType m_passType = PassType::Generic;
		uint32_t m_materialDescriptorSetIndex = std::numeric_limits<uint32_t>::max();
		uint32_t m_entityInstanceDescriptorSetIndex = std::numeric_limits<uint32_t>::max();
		const RenderQueue::InstanceSet *m_curInstanceSet = nullptr;

		pragma::ShaderGameWorld *m_curShader = nullptr;
		uint32_t m_curVertexAnimationOffset = std::numeric_limits<uint32_t>::max();
		RenderPassStats *m_stats = nullptr;

		bool m_materialDescSetBound = false;
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
		bool BindMaterial(msys::CMaterial &mat);
		virtual bool BindEntity(CBaseEntity &ent);
		void SetDepthBias(float d, float delta);
		bool Render(pragma::geometry::CModelSubMesh &mesh, pragma::rendering::RenderMeshIndex meshIdx, const RenderQueue::InstanceSet *instanceSet = nullptr);
		pragma::ShaderGameWorld *GetCurrentShader();
		void UnbindShader();
		void SetCountNonOpaqueMaterialsOnly(bool b);
		prosper::Extent2D GetExtents() const;
		void RecordViewport();
		const util::RenderPassDrawInfo &GetRenderPassDrawInfo() const { return m_drawSceneInfo; }
	  protected:
		uint32_t Render(const pragma::rendering::RenderQueue &renderQueue, RenderPass pass, RenderPassStats *optStats = nullptr, std::optional<uint32_t> worldRenderQueueIndex = {});
		bool BindInstanceSet(pragma::ShaderGameWorld &shaderScene, const RenderQueue::InstanceSet *instanceSet = nullptr);
		void UnbindMaterial();
		void UnbindEntity();
		msys::MaterialIndex m_curMaterialIndex = std::numeric_limits<msys::MaterialIndex>::max();
		EntityIndex m_curEntityIndex = std::numeric_limits<EntityIndex>::max();

		ShaderProcessor m_shaderProcessor;

		prosper::Shader *m_curShader = nullptr;
		prosper::PipelineID m_curPipeline = std::numeric_limits<prosper::PipelineID>::max();
		pragma::ShaderGameWorld *m_shaderScene = nullptr;
		msys::CMaterial *m_curMaterial = nullptr;
		CBaseEntity *m_curEntity = nullptr;
		pragma::CRenderComponent *m_curRenderC = nullptr;
		std::vector<std::shared_ptr<pragma::geometry::ModelSubMesh>> *m_curEntityMeshList = nullptr;
		const RenderQueue::InstanceSet *m_curInstanceSet = nullptr;
		ShaderGameWorld::SceneFlags m_baseSceneFlags = ShaderGameWorld::SceneFlags::None;

		prosper::PipelineID m_prepassCurScenePipeline = std::numeric_limits<prosper::PipelineID>::max();
		bool m_prepassIsCurScenePipelineTranslucent = false;

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
		uint32_t Render(const pragma::rendering::RenderQueue &renderQueue, RenderPass renderPass, RenderPassStats *optStats = nullptr, std::optional<uint32_t> worldRenderQueueIndex = {});
		void BindLight(CLightComponent &light, uint32_t layerId);
	};

	class DLLCLIENT LightingStageRenderProcessor : public pragma::rendering::BaseRenderProcessor {
	  public:
		using pragma::rendering::BaseRenderProcessor::BaseRenderProcessor;
		uint32_t Render(const pragma::rendering::RenderQueue &renderQueue, RenderPassStats *optStats = nullptr, std::optional<uint32_t> worldRenderQueueIndex = {});
	};
	using namespace umath::scoped_enum::bitwise;
};
export {REGISTER_ENUM_FLAGS(pragma::rendering::BaseRenderProcessor::StateFlags)}
