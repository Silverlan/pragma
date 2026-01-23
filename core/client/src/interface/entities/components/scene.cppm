// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.scene;

export import :debug.enums;
export import :entities.components.render;
export import :rendering.draw_scene_info;
export import :rendering.entity_instance_index_buffer;
export import :rendering.occlusion_culling.octree;
export import :rendering.render_queue;
export import :rendering.shaders.textured_uniform_data;
export import :rendering.world_environment;

export class DLLCLIENT SceneRenderDesc {
  public:
	enum class OcclusionCullingMethod : uint8_t { BruteForce = 0, CHCPP, BSP, Octree, Inert };
	enum class RenderQueueId : uint8_t {
		// Note: The order should match the actual render order for performance reasons!
		Skybox = 0u,
		SkyboxTranslucent,
		World,
		WorldTranslucent,
		View,
		ViewTranslucent,
		Water,
		Glow,

		Count,
		Invalid = std::numeric_limits<uint8_t>::max()
	};
	using WorldMeshVisibility = std::vector<bool>;
	// Note: All arguments have to be thread safe for the duration of the render (except vp)
	static void AddRenderMeshesToRenderQueue(pragma::CRasterizationRendererComponent *optRasterizationRenderer, pragma::rendering::RenderFlags renderFlags, pragma::CRenderComponent &renderC,
	  const std::function<pragma::rendering::RenderQueue *(pragma::rendering::SceneRenderPass, bool)> &getRenderQueue, const pragma::CSceneComponent &scene, const pragma::CCameraComponent &cam, const Mat4 &vp, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull,
	  int32_t lodBias = 0, const std::function<void(pragma::rendering::RenderQueue &, const pragma::rendering::RenderQueueItem &)> &fOptInsertItemToQueue = nullptr,
	  pragma::GameShaderSpecializationConstantFlag baseSpecializationFlags = static_cast<pragma::GameShaderSpecializationConstantFlag>(0));
	// Note: All arguments have to be thread safe for the duration of the render (except vp)
	static void CollectRenderMeshesFromOctree(pragma::CRasterizationRendererComponent *optRasterizationRenderer, pragma::rendering::RenderFlags renderFlags, bool enableClipping, const OcclusionOctree<pragma::ecs::CBaseEntity *> &tree, const pragma::CSceneComponent &scene,
	  const pragma::CCameraComponent &cam, const Mat4 &vp, pragma::rendering::RenderMask renderMask, const std::function<pragma::rendering::RenderQueue *(pragma::rendering::SceneRenderPass, bool)> &getRenderQueue, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull,
	  const std::vector<pragma::util::BSPTree *> *bspTrees = nullptr, const std::vector<pragma::util::BSPTree::Node *> *bspLeafNodes = nullptr, int32_t lodBias = 0,
	  const std::function<bool(pragma::ecs::CBaseEntity &, const pragma::CSceneComponent &, pragma::rendering::RenderFlags)> &shouldConsiderEntity = nullptr,
	  pragma::GameShaderSpecializationConstantFlag baseSpecializationFlags = static_cast<pragma::GameShaderSpecializationConstantFlag>(0));
	static bool ShouldConsiderEntity(pragma::ecs::CBaseEntity &ent, const pragma::CSceneComponent &scene, pragma::rendering::RenderFlags renderFlags, pragma::rendering::RenderMask renderMask);
	static bool ShouldCull(pragma::ecs::CBaseEntity &ent, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull);
	static bool ShouldCull(pragma::CRenderComponent &renderC, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull);
	static bool ShouldCull(pragma::CRenderComponent &renderC, pragma::rendering::RenderMeshIndex meshIdx, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull);
	static bool ShouldCull(const Vector3 &min, const Vector3 &max, const std::vector<pragma::math::Plane> &frustumPlanes);
	static uint32_t GetActiveRenderQueueThreadCount();
	static bool AssertRenderQueueThreadInactive();

	SceneRenderDesc(pragma::CSceneComponent &scene);
	~SceneRenderDesc();

	void BuildRenderQueues(const pragma::rendering::DrawSceneInfo &drawSceneInfo, const std::function<void()> &fBuildAdditionalQueues);
	void BuildRenderQueueInstanceLists(pragma::rendering::RenderQueue &renderQueue);

	bool IsWorldMeshVisible(uint32_t worldRenderQueueIndex, pragma::rendering::RenderMeshIndex meshIdx) const;

	void WaitForWorldRenderQueues() const;

	RenderQueueId GetRenderQueueId(pragma::rendering::SceneRenderPass renderMode, bool translucent) const;
	pragma::rendering::RenderQueue *GetRenderQueue(pragma::rendering::SceneRenderPass renderMode, bool translucent);
	const pragma::rendering::RenderQueue *GetRenderQueue(pragma::rendering::SceneRenderPass renderMode, bool translucent) const;
	const std::vector<std::shared_ptr<const pragma::rendering::RenderQueue>> &GetWorldRenderQueues() const;
  private:
	void AddRenderMeshesToRenderQueue(pragma::CRasterizationRendererComponent *optRasterizationRenderer, pragma::rendering::RenderFlags renderFlags, pragma::CRenderComponent &renderC, const pragma::CSceneComponent &scene, const pragma::CCameraComponent &cam, const Mat4 &vp,
	  const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull, pragma::GameShaderSpecializationConstantFlag baseSpecializationFlags = static_cast<pragma::GameShaderSpecializationConstantFlag>(0));
	void CollectRenderMeshesFromOctree(pragma::CRasterizationRendererComponent *optRasterizationRenderer, pragma::rendering::RenderFlags renderFlags, bool enableClipping, const OcclusionOctree<pragma::ecs::CBaseEntity *> &tree, const pragma::CSceneComponent &scene,
	  const pragma::CCameraComponent &cam, const Mat4 &vp, pragma::rendering::RenderMask renderMask, const std::vector<pragma::math::Plane> &frustumPlanes, const std::vector<pragma::util::BSPTree *> *bspTrees = nullptr,
	  const std::vector<pragma::util::BSPTree::Node *> *bspLeafNodes = nullptr);

	std::vector<WorldMeshVisibility> m_worldMeshVisibility;
	std::array<std::shared_ptr<pragma::rendering::RenderQueue>, pragma::math::to_integral(RenderQueueId::Count)> m_renderQueues;
	std::vector<std::shared_ptr<const pragma::rendering::RenderQueue>> m_worldRenderQueues;
	std::atomic<bool> m_worldRenderQueuesReady = false;

	pragma::CSceneComponent &m_scene;
};

export namespace pragma {
	namespace cSceneComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_ACTIVE_CAMERA_CHANGED;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_BUILD_RENDER_QUEUES;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_RENDERER_CHANGED;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_POST_RENDER_PREPASS;
	}
	class DLLCLIENT CSceneComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		friend SceneRenderDesc;
		enum class FRenderSetting : uint32_t { None = 0, Unlit = 1 };

		enum class StateFlags : uint32_t { None = 0u, ValidRenderer = 1u, HasParentScene = ValidRenderer << 1u };

		// Note: Scene index is *not* unique, a child-scene will share its index with its parent!
		using SceneIndex = uint8_t;
		using SceneFlags = uint32_t;
		struct DLLCLIENT CreateInfo {
			CreateInfo();
			prosper::SampleCountFlags sampleCount;
		};

		static CSceneComponent *Create(const CreateInfo &createInfo, CSceneComponent *optParent = nullptr);
		static CSceneComponent *GetByIndex(SceneIndex sceneIndex);
		static SceneFlags GetSceneFlag(SceneIndex sceneIndex);
		static SceneIndex GetSceneIndex(SceneFlags flag);
		static const std::shared_ptr<rendering::EntityInstanceIndexBuffer> &GetEntityInstanceIndexBuffer();
		static void UpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, const rendering::RenderQueue &renderQueue, rendering::RenderPassStats *optStats = nullptr);

		CSceneComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual ~CSceneComponent() override;

		virtual void InitializeLuaObject(lua::State *l) override;

		void Setup(const CreateInfo &createInfo, SceneIndex sceneIndex);
		const ComponentHandle<CCameraComponent> &GetActiveCamera() const;
		ComponentHandle<CCameraComponent> &GetActiveCamera();
		void SetActiveCamera(CCameraComponent &cam);
		void SetActiveCamera();

		void SetExclusionRenderMask(rendering::RenderMask renderMask);
		rendering::RenderMask GetExclusionRenderMask() const;
		void SetInclusionRenderMask(rendering::RenderMask renderMask);
		rendering::RenderMask GetInclusionRenderMask() const;

		void Link(const CSceneComponent &other, bool linkCamera = true);
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		void Resize(uint32_t width, uint32_t height, bool reload = false);

		void ReloadRenderTarget(uint32_t width, uint32_t height);

		void UpdateBuffers(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		const std::shared_ptr<prosper::IBuffer> &GetRenderSettingsBuffer() const;
		RenderSettings &GetRenderSettings();
		const RenderSettings &GetRenderSettings() const;
		const std::shared_ptr<prosper::IBuffer> &GetCameraBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetViewCameraBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetFogBuffer() const;
		const std::shared_ptr<prosper::IDescriptorSetGroup> &GetCameraDescriptorSetGroup(prosper::PipelineBindPoint bindPoint = prosper::PipelineBindPoint::Graphics) const;
		const std::shared_ptr<prosper::IDescriptorSetGroup> &GetViewCameraDescriptorSetGroup() const;
		prosper::IDescriptorSet *GetCameraDescriptorSetGraphics() const;
		prosper::IDescriptorSet *GetCameraDescriptorSetCompute() const;
		prosper::IDescriptorSet *GetViewCameraDescriptorSet() const;
		const std::shared_ptr<prosper::IDescriptorSetGroup> &GetFogDescriptorSetGroup() const;

		rendering::WorldEnvironment *GetWorldEnvironment() const;
		void SetWorldEnvironment(rendering::WorldEnvironment &env);
		void ClearWorldEnvironment();

		void LinkWorldEnvironment(CSceneComponent &other);
		template<typename TCPPM>
		void SetLightMap(TCPPM &lightMapC);

		template<typename TCPPM>
		void SetRenderer(TCPPM *renderer);
		template<typename TCPPM>
		TCPPM *GetRenderer();
		template<typename TCPPM>
		const TCPPM *GetRenderer() const;

		SceneDebugMode GetDebugMode() const;
		void SetDebugMode(SceneDebugMode debugMode);

		SceneRenderDesc &GetSceneRenderDesc();
		const SceneRenderDesc &GetSceneRenderDesc() const;

		void SetParticleSystemColorFactor(const Vector4 &colorFactor);
		const Vector4 &GetParticleSystemColorFactor() const;

		template<typename TCPPM>
		TCPPM *FindOcclusionCuller();
		template<typename TCPPM>
		const TCPPM *FindOcclusionCuller() const;
		SceneIndex GetSceneIndex() const;
		bool IsValid() const;
		CSceneComponent *GetParentScene();
		const CSceneComponent *GetParentScene() const { return const_cast<CSceneComponent *>(this)->GetParentScene(); }

		void BuildRenderQueues(const rendering::DrawSceneInfo &drawSceneInfo);

		const std::vector<ComponentHandle<CLightComponent>> &GetPreviouslyVisibleShadowedLights() const { return m_previouslyVisibleShadowedLights; }
		void SwapPreviouslyVisibleLights(std::vector<ComponentHandle<CLightComponent>> &&components) { std::swap(m_previouslyVisibleShadowedLights, components); }

		void RecordRenderCommandBuffers(const rendering::DrawSceneInfo &drawSceneInfo);
		void UpdateRenderData();
	  private:
		void InitializeShadowDescriptorSet();
		void UpdateRendererLightMap();
		// CSM Data
		struct DLLCLIENT CSMCascadeDescriptor {
			CSMCascadeDescriptor();
			//Vulkan::SwapDescriptorBuffer descBuffer; // prosper TODO
		};
		std::vector<std::unique_ptr<CSMCascadeDescriptor>> m_csmDescriptors;

		SceneIndex m_sceneIndex = std::numeric_limits<SceneIndex>::max();

		std::shared_ptr<prosper::IDescriptorSetGroup> m_camDescSetGroupGraphics = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_camDescSetGroupCompute = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_camViewDescSetGroup = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_shadowDsg = nullptr;

		std::vector<ComponentHandle<CLightComponent>> m_previouslyVisibleShadowedLights;
		ComponentHandle<BaseEntityComponent> m_lightMap = {};
		ComponentHandle<CCameraComponent> m_camera = {};
		std::shared_ptr<prosper::IBuffer> m_cameraBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_cameraViewBuffer = nullptr;

		std::shared_ptr<prosper::IBuffer> m_renderSettingsBuffer = nullptr;
		RenderSettings m_renderSettings = {};
		CameraData m_cameraData = {};
		SceneDebugMode m_debugMode = SceneDebugMode::None;
		Vector4 m_particleSystemColorFactor {1.f, 1.f, 1.f, 1.f};

		rendering::RenderMask m_exclusionRenderMask = rendering::RenderMask::None;
		rendering::RenderMask m_inclusionRenderMask = rendering::RenderMask::None;

		// Fog
		FogData m_fogData = {};
		std::shared_ptr<prosper::IBuffer> m_fogBuffer = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_fogDescSetGroup = nullptr;

		mutable std::vector<CallbackHandle> m_envCallbacks;
		mutable std::shared_ptr<rendering::WorldEnvironment> m_worldEnvironment;
		CallbackHandle m_cbFogCallback = {};
		CallbackHandle m_cbLink {};

		StateFlags m_stateFlags = StateFlags::None;
		ComponentHandle<BaseEntityComponent> m_renderer = ComponentHandle<BaseEntityComponent> {};
		SceneRenderDesc m_sceneRenderDesc;

		void UpdateCameraBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, bool bView = false);
		void UpdateRenderSettings();

		// Light Sources
		void InitializeRenderSettingsBuffer();
		void InitializeCameraBuffer();
		void InitializeFogBuffer();
		void InitializeDescriptorSetLayouts();
		void InitializeSwapDescriptorBuffers();
	};

	// Events

	struct DLLCLIENT CEDrawSceneInfo : public ComponentEvent {
		CEDrawSceneInfo(const rendering::DrawSceneInfo &drawSceneInfo);
		virtual void PushArguments(lua::State *l) override;
		const rendering::DrawSceneInfo &drawSceneInfo;
	};
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::CSceneComponent::FRenderSetting)
}
export {
	REGISTER_ENUM_FLAGS(pragma::CSceneComponent::StateFlags)
}

export class DLLCLIENT CScene : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
