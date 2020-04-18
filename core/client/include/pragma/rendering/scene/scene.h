#ifndef __SCENE_H__
#define __SCENE_H__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/c_ssao.hpp"
#include "pragma/rendering/c_prepass.hpp"
#include "pragma/rendering/c_forwardplus.hpp"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/shaders/world/c_shader_textured_uniform_data.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree.hpp"
#include <memory>
#include <sharedutils/util_weak_handle.hpp>
#include <unordered_set>

namespace pragma {class Shader; class ShaderTextured3DBase; class OcclusionCullingHandler; struct OcclusionMeshInfo;};
namespace prosper {class BlurSet; class RenderPass; class Fence; class DescriptorSet;};
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
#pragma warning(push)
#pragma warning(disable : 4251)
namespace pragma
{
	class CLightComponent; class CLightMapComponent; class CCameraComponent; class CParticleSystemComponent; namespace rendering {class BaseRenderer; class HDRData;};
	class COcclusionCullerComponent;
};
class CBaseEntity;
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
	using SceneIndex = uint8_t;
	struct DLLCLIENT CreateInfo
	{
		CreateInfo(uint32_t width,uint32_t height);
		uint32_t width;
		uint32_t height;
		Anvil::SampleCountFlagBits sampleCount;
	};

	static std::shared_ptr<Scene> Create(const CreateInfo &createInfo,Scene *optParent=nullptr);
	static Scene *GetByIndex(SceneIndex sceneIndex);

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

	void InitializeRenderTarget();

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
	prosper::DescriptorSet *GetCameraDescriptorSetGraphics() const;
	prosper::DescriptorSet *GetCameraDescriptorSetCompute() const;
	prosper::DescriptorSet *GetViewCameraDescriptorSet() const;
	const std::shared_ptr<prosper::DescriptorSetGroup> &GetFogDescriptorSetGroup() const;

	WorldEnvironment *GetWorldEnvironment() const;
	void SetWorldEnvironment(WorldEnvironment &env);
	void ClearWorldEnvironment();

	void LinkWorldEnvironment(Scene &other);
	void SetLightMap(pragma::CLightMapComponent &lightMapC);

	void SetRenderer(const std::shared_ptr<pragma::rendering::BaseRenderer> &renderer);
	pragma::rendering::BaseRenderer *GetRenderer();

	pragma::COcclusionCullerComponent *FindOcclusionCuller();
	SceneIndex GetSceneIndex() const;
	bool IsValid() const;
private:
	Scene(const CreateInfo &createInfo,SceneIndex sceneIndex);
	// CSM Data
	struct DLLCLIENT CSMCascadeDescriptor
	{
		CSMCascadeDescriptor();
		//Vulkan::SwapDescriptorBuffer descBuffer; // prosper TODO
	};
	std::vector<std::unique_ptr<CSMCascadeDescriptor>> m_csmDescriptors;

	SceneIndex m_sceneIndex = std::numeric_limits<SceneIndex>::max();

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
