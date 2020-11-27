/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SCENE_COMPONENT_HPP__
#define __C_SCENE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/c_shader_textured_uniform_data.hpp"
#include "pragma/rendering/c_renderflags.h"
#include "pragma/rendering/render_mesh_collection_handler.hpp"
#include <pragma/entities/components/base_entity_component.hpp>
#include <shader/prosper_descriptor_array_manager.hpp>

namespace pragma
{
	class CSceneComponent;
	class CLightMapComponent;
	class EntityComponentManager;
	class CCameraComponent;
	class OcclusionCullingHandler;
	class CParticleSystemComponent;
	class COcclusionCullerComponent;
	struct OcclusionMeshInfo;
	using RenderMeshIndex = uint32_t;
	namespace rendering {class RenderQueue; class BaseRenderer;};
};
class DLLCLIENT SceneRenderDesc
{
public:
	enum class OcclusionCullingMethod : uint8_t
	{
		BruteForce = 0,
		CHCPP,
		BSP,
		Octree,
		Inert
	};
	enum class RenderQueueId : uint8_t
	{
		// Note: The order should match the actual render order for performance reasons!
		Skybox = 0u,
		SkyboxTranslucent,
		World,
		WorldTranslucent,
		View,
		ViewTranslucent,
		Water,

		Count,
		Invalid = std::numeric_limits<uint8_t>::max()
	};
	using WorldMeshVisibility = std::vector<bool>;
	static void AddRenderMeshesToRenderQueue(
		const util::DrawSceneInfo &drawSceneInfo,pragma::CRenderComponent &renderC,
		const std::function<pragma::rendering::RenderQueue*(RenderMode,bool)> &getRenderQueue,
		const pragma::CSceneComponent &scene,const pragma::CCameraComponent &cam,const Mat4 &vp,const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull,
		int32_t lodBias=0
	);
	static void CollectRenderMeshesFromOctree(
		const util::DrawSceneInfo &drawSceneInfo,const OcclusionOctree<CBaseEntity*> &tree,const pragma::CSceneComponent &scene,const pragma::CCameraComponent &cam,const Mat4 &vp,FRender renderFlags,
		const std::function<pragma::rendering::RenderQueue*(RenderMode,bool)> &getRenderQueue,
		const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull,const std::vector<util::BSPTree::Node*> *bspLeafNodes=nullptr,
		int32_t lodBias=0
	);
	static bool ShouldConsiderEntity(CBaseEntity &ent,const pragma::CSceneComponent &scene,const Vector3 &camOrigin,FRender renderFlags);
	static bool ShouldCull(CBaseEntity &ent,const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull);
	static bool ShouldCull(pragma::CRenderComponent &renderC,const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull);
	static bool ShouldCull(pragma::CRenderComponent &renderC,pragma::RenderMeshIndex meshIdx,const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull);
	static bool ShouldCull(const Vector3 &min,const Vector3 &max,const std::vector<Plane> &frustumPlanes);

	SceneRenderDesc(pragma::CSceneComponent &scene);
	~SceneRenderDesc();
	const pragma::OcclusionCullingHandler &GetOcclusionCullingHandler() const;
	pragma::OcclusionCullingHandler &GetOcclusionCullingHandler();
	void SetOcclusionCullingHandler(const std::shared_ptr<pragma::OcclusionCullingHandler> &handler);
	void SetOcclusionCullingMethod(OcclusionCullingMethod method);
	void ReloadOcclusionCullingHandler();

	void BuildRenderQueues(const util::DrawSceneInfo &drawSceneInfo);
	void BuildRenderQueueInstanceLists(pragma::rendering::RenderQueue &renderQueue);

	bool IsWorldMeshVisible(uint32_t worldRenderQueueIndex,pragma::RenderMeshIndex meshIdx) const;

	// Culled objects
	const std::vector<pragma::OcclusionMeshInfo> &GetCulledMeshes() const;
	std::vector<pragma::OcclusionMeshInfo> &GetCulledMeshes();
	const std::vector<pragma::CParticleSystemComponent*> &GetCulledParticles() const;
	std::vector<pragma::CParticleSystemComponent*> &GetCulledParticles();

	pragma::rendering::RenderMeshCollectionHandler &GetRenderMeshCollectionHandler();
	const pragma::rendering::RenderMeshCollectionHandler &GetRenderMeshCollectionHandler() const;
	
	void WaitForWorldRenderQueues() const;

	RenderQueueId GetRenderQueueId(RenderMode renderMode,bool translucent) const;
	pragma::rendering::RenderQueue *GetRenderQueue(RenderMode renderMode,bool translucent);
	const pragma::rendering::RenderQueue *GetRenderQueue(RenderMode renderMode,bool translucent) const;
	const std::vector<std::shared_ptr<const pragma::rendering::RenderQueue>> &GetWorldRenderQueues() const;
	pragma::rendering::CulledMeshData *GetRenderInfo(RenderMode mode) const;
private:
	void AddRenderMeshesToRenderQueue(
		const util::DrawSceneInfo &drawSceneInfo,pragma::CRenderComponent &renderC,const pragma::CSceneComponent &scene,const pragma::CCameraComponent &cam,const Mat4 &vp,
		const std::function<bool(const Vector3&,const Vector3&)> &fShouldCull
	);
	void CollectRenderMeshesFromOctree(
		const util::DrawSceneInfo &drawSceneInfo,const OcclusionOctree<CBaseEntity*> &tree,const pragma::CSceneComponent &scene,const pragma::CCameraComponent &cam,const Mat4 &vp,FRender renderFlags,
		const std::vector<Plane> &frustumPlanes,const std::vector<util::BSPTree::Node*> *bspLeafNodes=nullptr
	);

	// TODO: Remove these, they're obsolete
	std::shared_ptr<pragma::OcclusionCullingHandler> m_occlusionCullingHandler = nullptr;
	pragma::rendering::RenderMeshCollectionHandler m_renderMeshCollectionHandler = {};

	std::vector<WorldMeshVisibility> m_worldMeshVisibility;
	std::array<std::shared_ptr<pragma::rendering::RenderQueue>,umath::to_integral(RenderQueueId::Count)> m_renderQueues;
	std::vector<std::shared_ptr<const pragma::rendering::RenderQueue>> m_worldRenderQueues;
	std::atomic<bool> m_worldRenderQueuesReady = false;

	pragma::CSceneComponent &m_scene;
};

#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLCLIENT ShaderMeshContainer
{
	ShaderMeshContainer(pragma::ShaderTextured3DBase *shader);
	ShaderMeshContainer(ShaderMeshContainer&)=delete;
	ShaderMeshContainer &operator=(const ShaderMeshContainer &other)=delete;
	::util::WeakHandle<prosper::Shader> shader = {};
	std::vector<std::unique_ptr<RenderSystem::MaterialMeshContainer>> containers;
};
#pragma warning(pop)

class WorldEnvironment;
namespace pragma
{
	namespace rendering {class EntityInstanceIndexBuffer;};
	class CLightComponent;
	enum class SceneDebugMode : uint32_t
	{
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
		Emission
	};
	class DLLCLIENT CSceneComponent final
		: public BaseEntityComponent
	{
	public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		static ComponentEventId EVENT_ON_ACTIVE_CAMERA_CHANGED;
		static ComponentEventId EVENT_ON_BUILD_RENDER_QUEUES;
		static ComponentEventId EVENT_POST_RENDER_PREPASS;

		friend SceneRenderDesc;
		enum class FRenderSetting : uint32_t
		{
			None = 0,
			Unlit = 1
		};

		using SceneIndex = uint8_t;
		using SceneFlags = uint32_t;
		struct DLLCLIENT CreateInfo
		{
			CreateInfo();
			prosper::SampleCountFlags sampleCount;
		};

		static CSceneComponent *Create(const CreateInfo &createInfo,CSceneComponent *optParent=nullptr);
		static CSceneComponent *GetByIndex(SceneIndex sceneIndex);
		static SceneFlags GetSceneFlag(SceneIndex sceneIndex);
		static SceneIndex GetSceneIndex(SceneFlags flag);
		static const std::shared_ptr<rendering::EntityInstanceIndexBuffer> &GetEntityInstanceIndexBuffer();

		CSceneComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual ~CSceneComponent() override;

		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		void Setup(const CreateInfo &createInfo,SceneIndex sceneIndex);
		const util::WeakHandle<pragma::CCameraComponent> &GetActiveCamera() const;
		util::WeakHandle<pragma::CCameraComponent> &GetActiveCamera();
		void SetActiveCamera(pragma::CCameraComponent &cam);
		void SetActiveCamera();

		void Link(const CSceneComponent &other);
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		void Resize(uint32_t width,uint32_t height,bool reload=false);

		void ReloadRenderTarget(uint32_t width,uint32_t height);

		void UpdateBuffers(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		const std::shared_ptr<prosper::IBuffer> &GetRenderSettingsBuffer() const;
		pragma::RenderSettings &GetRenderSettings();
		const pragma::RenderSettings &GetRenderSettings() const;
		const std::shared_ptr<prosper::IBuffer> &GetCameraBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetViewCameraBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetFogBuffer() const;
		const std::shared_ptr<prosper::IDescriptorSetGroup> &GetCameraDescriptorSetGroup(prosper::PipelineBindPoint bindPoint=prosper::PipelineBindPoint::Graphics) const;
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

		void SetRenderer(const std::shared_ptr<pragma::rendering::BaseRenderer> &renderer);
		pragma::rendering::BaseRenderer *GetRenderer();
		const pragma::rendering::BaseRenderer *GetRenderer() const;

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

		void BuildRenderQueues(const util::DrawSceneInfo &drawSceneInfo);

		const std::vector<util::WeakHandle<pragma::CLightComponent>> &GetPreviouslyVisibleShadowedLights() const {return m_previouslyVisibleShadowedLights;}
		void SwapPreviouslyVisibleLights(std::vector<util::WeakHandle<pragma::CLightComponent>> &&components) {std::swap(m_previouslyVisibleShadowedLights,components);}
	private:
		void InitializeShadowDescriptorSet();
		void UpdateRendererLightMap();
		// CSM Data
		struct DLLCLIENT CSMCascadeDescriptor
		{
			CSMCascadeDescriptor();
			//Vulkan::SwapDescriptorBuffer descBuffer; // prosper TODO
		};
		std::vector<std::unique_ptr<CSMCascadeDescriptor>> m_csmDescriptors;

		SceneIndex m_sceneIndex = std::numeric_limits<SceneIndex>::max();
	
		std::shared_ptr<prosper::IDescriptorSetGroup> m_camDescSetGroupGraphics = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_camDescSetGroupCompute = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_camViewDescSetGroup = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_shadowDsg = nullptr;

		std::vector<util::WeakHandle<pragma::CLightComponent>> m_previouslyVisibleShadowedLights;
		util::WeakHandle<pragma::CLightMapComponent> m_lightMap = {};
		util::WeakHandle<pragma::CCameraComponent> m_camera = {};
		std::shared_ptr<prosper::IBuffer> m_cameraBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_cameraViewBuffer = nullptr;

		std::shared_ptr<prosper::IBuffer> m_renderSettingsBuffer = nullptr;
		pragma::RenderSettings m_renderSettings = {};
		pragma::CameraData m_cameraData = {};
		SceneDebugMode m_debugMode = SceneDebugMode::None;
		Vector4 m_particleSystemColorFactor {1.f,1.f,1.f,1.f};

		// Fog
		pragma::FogData m_fogData = {};
		std::shared_ptr<prosper::IBuffer> m_fogBuffer = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_fogDescSetGroup = nullptr;

		mutable std::vector<CallbackHandle> m_envCallbacks;
		mutable std::shared_ptr<WorldEnvironment> m_worldEnvironment;
		CallbackHandle m_cbFogCallback = {};
		CallbackHandle m_cbLink {};

		bool m_bValid = false;
		std::shared_ptr<pragma::rendering::BaseRenderer> m_renderer = nullptr;
		SceneRenderDesc m_sceneRenderDesc;

		void UpdateCameraBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,bool bView=false);
		void UpdateRenderSettings();

		// Light Sources
		void InitializeRenderSettingsBuffer();
		void InitializeCameraBuffer();
		void InitializeFogBuffer();
		void InitializeDescriptorSetLayouts();
		void InitializeSwapDescriptorBuffers();
	};

	// Events

	struct DLLCLIENT CEDrawSceneInfo
		: public ComponentEvent
	{
		CEDrawSceneInfo(const util::DrawSceneInfo &drawSceneInfo);
		virtual void PushArguments(lua_State *l) override;
		const util::DrawSceneInfo &drawSceneInfo;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CSceneComponent::FRenderSetting);

class EntityHandle;
class DLLCLIENT CScene
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
