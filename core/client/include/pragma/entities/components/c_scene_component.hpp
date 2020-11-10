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
	namespace rendering {class BaseRenderer;};
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
	SceneRenderDesc(pragma::CSceneComponent &scene);
	~SceneRenderDesc();
	const pragma::OcclusionCullingHandler &GetOcclusionCullingHandler() const;
	pragma::OcclusionCullingHandler &GetOcclusionCullingHandler();
	void SetOcclusionCullingHandler(const std::shared_ptr<pragma::OcclusionCullingHandler> &handler);
	void SetOcclusionCullingMethod(OcclusionCullingMethod method);
	void ReloadOcclusionCullingHandler();
	void PrepareRendering(pragma::CSceneComponent &scene,RenderMode mode,FRender renderFlags,bool bUpdateTranslucentMeshes=false,bool bUpdateGlowMeshes=false);

	// Culled objects
	const std::vector<pragma::OcclusionMeshInfo> &GetCulledMeshes() const;
	std::vector<pragma::OcclusionMeshInfo> &GetCulledMeshes();
	const std::vector<pragma::CParticleSystemComponent*> &GetCulledParticles() const;
	std::vector<pragma::CParticleSystemComponent*> &GetCulledParticles();

	pragma::rendering::RenderMeshCollectionHandler &GetRenderMeshCollectionHandler();
	const pragma::rendering::RenderMeshCollectionHandler &GetRenderMeshCollectionHandler() const;

	void PerformOcclusionCulling();
	void CollectRenderObjects(FRender renderFlags);
	pragma::rendering::CulledMeshData *GetRenderInfo(RenderMode mode) const;
private:
	std::shared_ptr<pragma::OcclusionCullingHandler> m_occlusionCullingHandler = nullptr;
	pragma::rendering::RenderMeshCollectionHandler m_renderMeshCollectionHandler = {};
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
	class DLLCLIENT CSceneComponent final
		: public BaseEntityComponent
	{
	public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		static ComponentEventId EVENT_ON_ACTIVE_CAMERA_CHANGED;

		friend SceneRenderDesc;
		enum class FRenderSetting : uint32_t
		{
			None = 0,
			Unlit = 1
		};

		enum class DebugMode : uint32_t
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

		using SceneIndex = uint8_t;
		struct DLLCLIENT CreateInfo
		{
			CreateInfo();
			prosper::SampleCountFlags sampleCount;
		};

		static CSceneComponent *Create(const CreateInfo &createInfo,CSceneComponent *optParent=nullptr);
		static CSceneComponent *GetByIndex(SceneIndex sceneIndex);
		static uint32_t GetSceneFlag(SceneIndex sceneIndex);
		static SceneIndex GetSceneIndex(uint32_t flag);

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

		DebugMode GetDebugMode() const;
		void SetDebugMode(DebugMode debugMode);

		SceneRenderDesc &GetSceneRenderDesc();
		const SceneRenderDesc &GetSceneRenderDesc() const;

		void SetParticleSystemColorFactor(const Vector4 &colorFactor);
		const Vector4 &GetParticleSystemColorFactor() const;

		pragma::COcclusionCullerComponent *FindOcclusionCuller();
		SceneIndex GetSceneIndex() const;
		bool IsValid() const;
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

		util::WeakHandle<pragma::CLightMapComponent> m_lightMap = {};
		util::WeakHandle<pragma::CCameraComponent> m_camera = {};
		std::shared_ptr<prosper::IBuffer> m_cameraBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_cameraViewBuffer = nullptr;

		std::shared_ptr<prosper::IBuffer> m_renderSettingsBuffer = nullptr;
		pragma::RenderSettings m_renderSettings = {};
		pragma::CameraData m_cameraData = {};
		DebugMode m_debugMode = DebugMode::None;
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
