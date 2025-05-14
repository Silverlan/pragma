/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SCENE_COMPONENT_HPP__
#define __C_SCENE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/rendering/shaders/world/c_shader_textured_uniform_data.hpp"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/rendering/c_renderflags.h"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include <pragma/util/util_bsp_tree.hpp>
#include <pragma/entities/components/base_entity_component.hpp>
#include <shader/prosper_descriptor_array_manager.hpp>
#include <mathutil/plane.hpp>
#include <cinttypes>

namespace pragma {
	class CSceneComponent;
	class CRenderComponent;
	class CLightMapComponent;
	class EntityComponentManager;
	class CCameraComponent;
	class OcclusionCullingHandler;
	class CParticleSystemComponent;
	class COcclusionCullerComponent;
	class CRasterizationRendererComponent;
	class CRendererComponent;
	class ShaderGameWorldLightingPass;
	struct OcclusionMeshInfo;
	using RenderMeshIndex = uint32_t;
	namespace rendering {
		class RenderQueue;
		struct RenderQueueItem;
	};
	enum class GameShaderSpecializationConstantFlag : uint32_t;
};
template<class T>
class OcclusionOctree;

namespace util {
	struct DrawSceneInfo;
};
class DLLCLIENT SceneRenderDesc {
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
	static void AddRenderMeshesToRenderQueue(pragma::CRasterizationRendererComponent *optRasterizationRenderer, RenderFlags renderFlags, pragma::CRenderComponent &renderC, const std::function<pragma::rendering::RenderQueue *(pragma::rendering::SceneRenderPass, bool)> &getRenderQueue,
	  const pragma::CSceneComponent &scene, const pragma::CCameraComponent &cam, const Mat4 &vp, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull, int32_t lodBias = 0,
	  const std::function<void(pragma::rendering::RenderQueue &, const pragma::rendering::RenderQueueItem &)> &fOptInsertItemToQueue = nullptr, pragma::GameShaderSpecializationConstantFlag baseSpecializationFlags = static_cast<pragma::GameShaderSpecializationConstantFlag>(0));
	// Note: All arguments have to be thread safe for the duration of the render (except vp)
	static void CollectRenderMeshesFromOctree(pragma::CRasterizationRendererComponent *optRasterizationRenderer, RenderFlags renderFlags, bool enableClipping, const OcclusionOctree<CBaseEntity *> &tree, const pragma::CSceneComponent &scene, const pragma::CCameraComponent &cam,
	  const Mat4 &vp, pragma::rendering::RenderMask renderMask, const std::function<pragma::rendering::RenderQueue *(pragma::rendering::SceneRenderPass, bool)> &getRenderQueue, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull,
	  const std::vector<util::BSPTree *> *bspTrees = nullptr, const std::vector<util::BSPTree::Node *> *bspLeafNodes = nullptr, int32_t lodBias = 0, const std::function<bool(CBaseEntity &, const pragma::CSceneComponent &, RenderFlags)> &shouldConsiderEntity = nullptr,
	  pragma::GameShaderSpecializationConstantFlag baseSpecializationFlags = static_cast<pragma::GameShaderSpecializationConstantFlag>(0));
	static bool ShouldConsiderEntity(CBaseEntity &ent, const pragma::CSceneComponent &scene, RenderFlags renderFlags, pragma::rendering::RenderMask renderMask);
	static bool ShouldCull(CBaseEntity &ent, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull);
	static bool ShouldCull(pragma::CRenderComponent &renderC, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull);
	static bool ShouldCull(pragma::CRenderComponent &renderC, pragma::RenderMeshIndex meshIdx, const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull);
	static bool ShouldCull(const Vector3 &min, const Vector3 &max, const std::vector<umath::Plane> &frustumPlanes);
	static uint32_t GetActiveRenderQueueThreadCount();
	static bool AssertRenderQueueThreadInactive();

	SceneRenderDesc(pragma::CSceneComponent &scene);
	~SceneRenderDesc();
	const pragma::OcclusionCullingHandler &GetOcclusionCullingHandler() const;
	pragma::OcclusionCullingHandler &GetOcclusionCullingHandler();
	void SetOcclusionCullingHandler(const std::shared_ptr<pragma::OcclusionCullingHandler> &handler);
	void SetOcclusionCullingMethod(OcclusionCullingMethod method);
	void ReloadOcclusionCullingHandler();

	void BuildRenderQueues(const util::DrawSceneInfo &drawSceneInfo, const std::function<void()> &fBuildAdditionalQueues);
	void BuildRenderQueueInstanceLists(pragma::rendering::RenderQueue &renderQueue);

	bool IsWorldMeshVisible(uint32_t worldRenderQueueIndex, pragma::RenderMeshIndex meshIdx) const;

	void WaitForWorldRenderQueues() const;

	RenderQueueId GetRenderQueueId(pragma::rendering::SceneRenderPass renderMode, bool translucent) const;
	pragma::rendering::RenderQueue *GetRenderQueue(pragma::rendering::SceneRenderPass renderMode, bool translucent);
	const pragma::rendering::RenderQueue *GetRenderQueue(pragma::rendering::SceneRenderPass renderMode, bool translucent) const;
	const std::vector<std::shared_ptr<const pragma::rendering::RenderQueue>> &GetWorldRenderQueues() const;
  private:
	void AddRenderMeshesToRenderQueue(pragma::CRasterizationRendererComponent *optRasterizationRenderer, RenderFlags renderFlags, pragma::CRenderComponent &renderC, const pragma::CSceneComponent &scene, const pragma::CCameraComponent &cam, const Mat4 &vp,
	  const std::function<bool(const Vector3 &, const Vector3 &)> &fShouldCull, pragma::GameShaderSpecializationConstantFlag baseSpecializationFlags = static_cast<pragma::GameShaderSpecializationConstantFlag>(0));
	void CollectRenderMeshesFromOctree(pragma::CRasterizationRendererComponent *optRasterizationRenderer, RenderFlags renderFlags, bool enableClipping, const OcclusionOctree<CBaseEntity *> &tree, const pragma::CSceneComponent &scene, const pragma::CCameraComponent &cam, const Mat4 &vp,
	  pragma::rendering::RenderMask renderMask, const std::vector<umath::Plane> &frustumPlanes, const std::vector<util::BSPTree *> *bspTrees = nullptr, const std::vector<util::BSPTree::Node *> *bspLeafNodes = nullptr);

	// TODO: Remove these, they're obsolete
	std::shared_ptr<pragma::OcclusionCullingHandler> m_occlusionCullingHandler = nullptr;

	std::vector<WorldMeshVisibility> m_worldMeshVisibility;
	std::array<std::shared_ptr<pragma::rendering::RenderQueue>, umath::to_integral(RenderQueueId::Count)> m_renderQueues;
	std::vector<std::shared_ptr<const pragma::rendering::RenderQueue>> m_worldRenderQueues;
	std::atomic<bool> m_worldRenderQueuesReady = false;

	pragma::CSceneComponent &m_scene;
};

class WorldEnvironment;
struct RenderPassStats;
namespace pragma {
	namespace rendering {
		class EntityInstanceIndexBuffer;
	};
	class CLightComponent;
	enum class SceneDebugMode : uint32_t {
		None = 0,
		AmbientOcclusion,
		Albedo,
		Metalness,
		Roughness,
		DiffuseLighting,
		Normal,
		NormalMap,
		Reflectance,
		IBLPrefilter,
		IBLIrradiance,
		Emission,
		Lightmap,
		LightmapUv,
		Unlit,
		CsmShowCascades,
		ShadowMapDepth,
		ForwardPlusHeatmap,
		Specular,
		IndirectLightmap,
		DirectionalLightmap,

		Count
	};
	class DLLCLIENT CSceneComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		static ComponentEventId EVENT_ON_ACTIVE_CAMERA_CHANGED;
		static ComponentEventId EVENT_ON_BUILD_RENDER_QUEUES;
		static ComponentEventId EVENT_ON_RENDERER_CHANGED;
		static ComponentEventId EVENT_POST_RENDER_PREPASS;

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
		static void UpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, const rendering::RenderQueue &renderQueue, RenderPassStats *optStats = nullptr);

		CSceneComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual ~CSceneComponent() override;

		virtual void InitializeLuaObject(lua_State *l) override;

		void Setup(const CreateInfo &createInfo, SceneIndex sceneIndex);
		const ComponentHandle<pragma::CCameraComponent> &GetActiveCamera() const;
		ComponentHandle<pragma::CCameraComponent> &GetActiveCamera();
		void SetActiveCamera(pragma::CCameraComponent &cam);
		void SetActiveCamera();

		void SetExclusionRenderMask(::pragma::rendering::RenderMask renderMask);
		::pragma::rendering::RenderMask GetExclusionRenderMask() const;
		void SetInclusionRenderMask(::pragma::rendering::RenderMask renderMask);
		::pragma::rendering::RenderMask GetInclusionRenderMask() const;

		void Link(const CSceneComponent &other, bool linkCamera = true);
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		void Resize(uint32_t width, uint32_t height, bool reload = false);

		void ReloadRenderTarget(uint32_t width, uint32_t height);

		void UpdateBuffers(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		const std::shared_ptr<prosper::IBuffer> &GetRenderSettingsBuffer() const;
		pragma::RenderSettings &GetRenderSettings();
		const pragma::RenderSettings &GetRenderSettings() const;
		const std::shared_ptr<prosper::IBuffer> &GetCameraBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetViewCameraBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetFogBuffer() const;
		const std::shared_ptr<prosper::IDescriptorSetGroup> &GetCameraDescriptorSetGroup(prosper::PipelineBindPoint bindPoint = prosper::PipelineBindPoint::Graphics) const;
		const std::shared_ptr<prosper::IDescriptorSetGroup> &GetViewCameraDescriptorSetGroup() const;
		prosper::IDescriptorSet *GetCameraDescriptorSetGraphics() const;
		prosper::IDescriptorSet *GetCameraDescriptorSetCompute() const;
		prosper::IDescriptorSet *GetViewCameraDescriptorSet() const;
		const std::shared_ptr<prosper::IDescriptorSetGroup> &GetFogDescriptorSetGroup() const;

		WorldEnvironment *GetWorldEnvironment() const;
		void SetWorldEnvironment(WorldEnvironment &env);
		void ClearWorldEnvironment();

		void LinkWorldEnvironment(CSceneComponent &other);
		void SetLightMap(pragma::CLightMapComponent &lightMapC);

		void SetRenderer(CRendererComponent *renderer);
		CRendererComponent *GetRenderer();
		const CRendererComponent *GetRenderer() const;

		SceneDebugMode GetDebugMode() const;
		void SetDebugMode(SceneDebugMode debugMode);

		SceneRenderDesc &GetSceneRenderDesc();
		const SceneRenderDesc &GetSceneRenderDesc() const;

		void SetParticleSystemColorFactor(const Vector4 &colorFactor);
		const Vector4 &GetParticleSystemColorFactor() const;

		pragma::COcclusionCullerComponent *FindOcclusionCuller();
		const pragma::COcclusionCullerComponent *FindOcclusionCuller() const;
		SceneIndex GetSceneIndex() const;
		bool IsValid() const;
		CSceneComponent *GetParentScene();
		const CSceneComponent *GetParentScene() const { return const_cast<CSceneComponent *>(this)->GetParentScene(); }

		void BuildRenderQueues(const ::util::DrawSceneInfo &drawSceneInfo);

		const std::vector<ComponentHandle<pragma::CLightComponent>> &GetPreviouslyVisibleShadowedLights() const { return m_previouslyVisibleShadowedLights; }
		void SwapPreviouslyVisibleLights(std::vector<ComponentHandle<pragma::CLightComponent>> &&components) { std::swap(m_previouslyVisibleShadowedLights, components); }

		void RecordRenderCommandBuffers(const ::util::DrawSceneInfo &drawSceneInfo);
		void UpdateRenderData();
	  private:
		static float CalcLightMapPowExposure(pragma::CLightMapComponent &lightMapC);
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

		std::vector<ComponentHandle<pragma::CLightComponent>> m_previouslyVisibleShadowedLights;
		ComponentHandle<pragma::CLightMapComponent> m_lightMap = {};
		ComponentHandle<pragma::CCameraComponent> m_camera = {};
		std::shared_ptr<prosper::IBuffer> m_cameraBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_cameraViewBuffer = nullptr;

		std::shared_ptr<prosper::IBuffer> m_renderSettingsBuffer = nullptr;
		pragma::RenderSettings m_renderSettings = {};
		pragma::CameraData m_cameraData = {};
		SceneDebugMode m_debugMode = SceneDebugMode::None;
		Vector4 m_particleSystemColorFactor {1.f, 1.f, 1.f, 1.f};

		::pragma::rendering::RenderMask m_exclusionRenderMask = ::pragma::rendering::RenderMask::None;
		::pragma::rendering::RenderMask m_inclusionRenderMask = ::pragma::rendering::RenderMask::None;

		// Fog
		pragma::FogData m_fogData = {};
		std::shared_ptr<prosper::IBuffer> m_fogBuffer = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_fogDescSetGroup = nullptr;

		mutable std::vector<CallbackHandle> m_envCallbacks;
		mutable std::shared_ptr<WorldEnvironment> m_worldEnvironment;
		CallbackHandle m_cbFogCallback = {};
		CallbackHandle m_cbLink {};

		StateFlags m_stateFlags = StateFlags::None;
		ComponentHandle<CRendererComponent> m_renderer = ComponentHandle<CRendererComponent> {};
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
		CEDrawSceneInfo(const ::util::DrawSceneInfo &drawSceneInfo);
		virtual void PushArguments(lua_State *l) override;
		const ::util::DrawSceneInfo &drawSceneInfo;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CSceneComponent::FRenderSetting);
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CSceneComponent::StateFlags);

class DLLCLIENT CScene : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
