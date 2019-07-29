#ifndef __SCENE_H__
#define __SCENE_H__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/c_ssao.hpp"
#include "pragma/rendering/c_prepass.hpp"
#include "pragma/rendering/c_forwardplus.hpp"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/shaders/world/c_shader_textured_uniform_data.hpp"
#include <memory>
#include <sharedutils/util_weak_handle.hpp>
#include <unordered_set>

namespace pragma {class Shader; class ShaderTextured3D; class OcclusionCullingHandler; struct OcclusionMeshInfo;};
namespace prosper {class BlurSet; class RenderPass; class Fence;};
#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLCLIENT ShaderMeshContainer
{
	ShaderMeshContainer(pragma::ShaderTextured3D *shader);
	ShaderMeshContainer(ShaderMeshContainer&)=delete;
	ShaderMeshContainer &operator=(const ShaderMeshContainer &other)=delete;
	::util::WeakHandle<prosper::Shader> shader = {};
	std::vector<std::unique_ptr<RenderSystem::MaterialMeshContainer>> containers;
};
#pragma warning(pop)

class WorldEnvironment;
#pragma warning(push)
#pragma warning(disable : 4251)
namespace pragma {class CLightComponent; class CCameraComponent; class CParticleSystemComponent; namespace rendering {class BaseRenderer; class HDRData;};};
class DLLCLIENT Scene
	: public std::enable_shared_from_this<Scene>
{
public:
	enum class DLLCLIENT FRenderSetting : uint32_t
	{
		None = 0,
		Unlit = 1,
		SSAOEnabled = 2
	};
public:
	struct DLLCLIENT LightListInfo
	{
		void AddLightSource(pragma::CLightComponent &lightSource);
		void RemoveLightSource(pragma::CLightComponent &lightSource);
		std::vector<util::WeakHandle<pragma::CLightComponent>> lightSources;
		std::unordered_set<pragma::CLightComponent*> lightSourceLookupTable;
	};

	struct DLLCLIENT EntityListInfo
	{
		void AddEntity(CBaseEntity &ent);
		void RemoveEntity(CBaseEntity &ent);
		std::vector<EntityHandle> entities;
		std::unordered_set<CBaseEntity*> entityLookupTable;
	};

	struct DLLCLIENT CreateInfo
	{
		CreateInfo(uint32_t width,uint32_t height);
		uint32_t width;
		uint32_t height;
		Anvil::SampleCountFlagBits sampleCount;
	};

	static std::shared_ptr<Scene> Create(const CreateInfo &createInfo);

	//static void ClearLightCache();
	~Scene();
	Scene(Scene&)=delete;
	Scene &operator=(const Scene&)=delete;
	//const Vulkan::DescriptorSet *GetCSMShadowDescriptorSet(uint32_t layer,uint32_t swapIdx=0); // prosper TODO
	//const Vulkan::Buffer *GetCSMShadowBuffer(uint32_t layer,uint32_t swapIdx=0); // prosper TODO

	const util::WeakHandle<pragma::CCameraComponent> &GetActiveCamera() const;
	util::WeakHandle<pragma::CCameraComponent> &GetActiveCamera();
	void SetActiveCamera(pragma::CCameraComponent &cam);
	void SetActiveCamera();

	void SetLights(const std::vector<pragma::CLightComponent*> &lights);
	void SetLights(const std::shared_ptr<LightListInfo> &lights);
	void AddLight(pragma::CLightComponent *light);
	void RemoveLight(pragma::CLightComponent *light);
	const std::shared_ptr<LightListInfo> &GetLightSourceListInfo() const;
	const std::vector<util::WeakHandle<pragma::CLightComponent>> &GetLightSources() const;
	std::vector<util::WeakHandle<pragma::CLightComponent>> &GetLightSources();
	bool HasLightSource(pragma::CLightComponent &lightSource) const;

	void InitializeRenderTarget();

	void SetEntities(const std::vector<CBaseEntity*> &ents);
	void SetEntities(const std::shared_ptr<EntityListInfo> &ents);
	void AddEntity(CBaseEntity &ent);
	void RemoveEntity(CBaseEntity &ent);
	const std::shared_ptr<EntityListInfo> &GetEntityListInfo() const;
	const std::vector<EntityHandle> &GetEntities() const;
	std::vector<EntityHandle> &GetEntities();
	bool HasEntity(CBaseEntity &ent) const;

	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	void Resize(uint32_t width,uint32_t height);

	void ReloadRenderTarget();

	void UpdateBuffers(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);
	const std::shared_ptr<prosper::Buffer> &GetRenderSettingsBuffer() const;
	pragma::RenderSettings &GetRenderSettings();
	const pragma::RenderSettings &GetRenderSettings() const;
	const std::shared_ptr<prosper::Buffer> &GetCameraBuffer() const;
	const std::shared_ptr<prosper::Buffer> &GetViewCameraBuffer() const;
	const std::shared_ptr<prosper::Buffer> &GetFogBuffer() const;
	const std::shared_ptr<prosper::DescriptorSetGroup> &GetCameraDescriptorSetGroup(vk::PipelineBindPoint bindPoint=vk::PipelineBindPoint::eGraphics) const;
	const std::shared_ptr<prosper::DescriptorSetGroup> &GetViewCameraDescriptorSetGroup() const;
	Anvil::DescriptorSet *GetCameraDescriptorSetGraphics() const;
	Anvil::DescriptorSet *GetCameraDescriptorSetCompute() const;
	Anvil::DescriptorSet *GetViewCameraDescriptorSet() const;
	const std::shared_ptr<prosper::DescriptorSetGroup> &GetFogDescriptorSetGroup() const;

	WorldEnvironment *GetWorldEnvironment() const;
	void SetWorldEnvironment(WorldEnvironment &env);
	void ClearWorldEnvironment();

	void LinkLightSources(Scene &other);
	void LinkEntities(Scene &other);
	void LinkWorldEnvironment(Scene &other);

	void SetRenderer(const std::shared_ptr<pragma::rendering::BaseRenderer> &renderer);
	pragma::rendering::BaseRenderer *GetRenderer();

	bool IsValid() const;
private:
	Scene(const CreateInfo &createInfo);
	static std::vector<Scene*> s_scenes;
	// CSM Data
	struct DLLCLIENT CSMCascadeDescriptor
	{
		CSMCascadeDescriptor();
		//Vulkan::SwapDescriptorBuffer descBuffer; // prosper TODO
	};
	std::vector<std::unique_ptr<CSMCascadeDescriptor>> m_csmDescriptors;

	// Render Target
	uint32_t m_width;
	uint32_t m_height;

	std::shared_ptr<prosper::DescriptorSetGroup> m_camDescSetGroupGraphics = nullptr;
	std::shared_ptr<prosper::DescriptorSetGroup> m_camDescSetGroupCompute = nullptr;
	std::shared_ptr<prosper::DescriptorSetGroup> m_camViewDescSetGroup = nullptr;

	util::WeakHandle<pragma::CCameraComponent> m_camera = {};
	std::shared_ptr<prosper::Buffer> m_cameraBuffer = nullptr;
	std::shared_ptr<prosper::Buffer> m_cameraViewBuffer = nullptr;

	std::shared_ptr<prosper::Buffer> m_renderSettingsBuffer = nullptr;
	pragma::RenderSettings m_renderSettings = {};
	pragma::CameraData m_cameraData = {};

	// Fog
	pragma::FogData m_fogData = {};
	std::shared_ptr<prosper::Buffer> m_fogBuffer = nullptr;
	std::shared_ptr<prosper::DescriptorSetGroup> m_fogDescSetGroup = nullptr;

	mutable std::vector<CallbackHandle> m_envCallbacks;
	mutable std::shared_ptr<WorldEnvironment> m_worldEnvironment;
	CallbackHandle m_cbFogCallback = {};

	bool m_bValid = false;
	std::shared_ptr<pragma::rendering::BaseRenderer> m_renderer = nullptr;

	std::shared_ptr<LightListInfo> m_lightSources = nullptr;
	std::shared_ptr<EntityListInfo> m_entityList = nullptr;

	void UpdateCameraBuffer(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,bool bView=false);
	void UpdateRenderSettings();

	// Light Sources
	void InitializeRenderSettingsBuffer();
	void InitializeCameraBuffer();
	void InitializeFogBuffer();
	void InitializeDescriptorSetLayouts();
	void InitializeSwapDescriptorBuffers();
};
REGISTER_BASIC_BITWISE_OPERATORS(Scene::FRenderSetting);
#pragma warning(pop)

#endif
