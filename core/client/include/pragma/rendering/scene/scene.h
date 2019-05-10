#ifndef __SCENE_H__
#define __SCENE_H__

#include "pragma/rendering/scene/camera.h"
#include "pragma/clientdefinitions.h"
#include "pragma/rendering/c_ssao.hpp"
#include "pragma/rendering/c_prepass.hpp"
#include "pragma/rendering/c_forwardplus.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured_uniform_data.hpp"
#include <memory>
#include <sharedutils/util_weak_handle.hpp>
#include <unordered_set>

namespace pragma {class Shader; class ShaderTextured3D; class OcclusionCullingHandler; class OcclusionMeshInfo;};
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
namespace pragma {class CParticleSystemComponent;};
class DLLCLIENT Scene
	: public std::enable_shared_from_this<Scene>
{
public:
	struct DLLCLIENT HDRInfo
	{
		HDRInfo();
		~HDRInfo();
		void UpdateExposure(prosper::Texture &srcTexture);
		bool Initialize(Scene &scene,uint32_t width,uint32_t height,Anvil::SampleCountFlagBits sampleCount,bool bEnableSSAO);
		bool InitializeDescriptorSets();

		void SwapIOTextures();
		//const Vulkan::Texture &GetInputTexture() const; // prosper TODO
		//const Vulkan::Texture &GetOutputTexture() const; // prosper TODO
		//const Vulkan::DescriptorSet &GetInputDescriptorSet() const; // prosper TODO
		bool BeginRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,prosper::RenderPass *customRenderPass=nullptr);
		bool EndRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);
		bool ResolveRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);
		//void BeginRenderPass(Vulkan::CommandBufferObject *drawCmd); // prosper TODO
		//void EndRenderPass(Vulkan::CommandBufferObject *drawCmd); // prosper TODO
		void ResetIOTextureIndex();
		bool BlitStagingRenderTargetToMainRenderTarget(prosper::CommandBuffer &cmdBuffer);
		bool BlitMainDepthBufferToSamplableDepthBuffer(prosper::CommandBuffer &cmdBuffer,std::function<void(prosper::CommandBuffer&)> &fTransitionSampleImgToTransferDst);

		SSAOInfo ssaoInfo;
		pragma::rendering::Prepass prepass;
		pragma::rendering::ForwardPlusInstance forwardPlusInstance;
		std::shared_ptr<prosper::Texture> bloomTexture = nullptr;
		std::shared_ptr<prosper::Texture> bloomBlurTexture = nullptr;
		std::shared_ptr<prosper::RenderTarget> bloomBlurRenderTarget = nullptr;
		std::shared_ptr<prosper::RenderTarget> hdrRenderTarget = nullptr;
		std::shared_ptr<prosper::RenderTarget> hdrStagingRenderTarget = nullptr;
		std::shared_ptr<prosper::RenderTarget> postHdrRenderTarget = nullptr;

		std::shared_ptr<prosper::BlurSet> bloomBlurSet = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupHdr = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupPostHdr = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupHdrResolve = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupHdrResolveStaging = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupDepth = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupDepthPostProcessing = nullptr;

		std::shared_ptr<prosper::RenderPass> rpIntermediate = nullptr;

		util::WeakHandle<prosper::Shader> shaderPPHdr = {};
		 // prosper TODOVulkan::MSAATexture texture; // Multi-Sample image // prosper TODO
		 // prosper TODOVulkan::MSAATexture textureBloom; // prosper TODO
		//Vulkan::Texture textureDepthSingleSample;
		//Vulkan::Texture textureSwap; // prosper TODO
		//Vulkan::CommandBuffer cmdBufferBloom;
		//std::function<void(Vulkan::CommandBufferObject*)> cmdBufferBloom; // prosper TODO
		/*Vulkan::DescriptorSet descSetSwap;
		Vulkan::DescriptorSet descSetSwap2;
		Vulkan::DescriptorSet descSetDepth;
		Vulkan::Framebuffer blurFramebuffer;
		Shader::GaussianBlur::BlurBuffer blurBuffer;
		Vulkan::Framebuffer framebuffer;
		Vulkan::Framebuffer framebufferSwap;
		Vulkan::Framebuffer framebufferSwap2;
		Vulkan::RenderPass renderPass;
		Vulkan::RenderPass renderPassSwap;
		const Vulkan::Texture &GetRenderTexture() const;
		const Vulkan::Texture &GetTargetTexture() const;
		const Vulkan::Texture &GetTargetBloomTexture() const;
		const Vulkan::Texture &GetTargetDepthTexture() const;*/ // prosper TODO
		Float exposure;
		Float max_exposure;
		std::array<float,3> luminescence;
	private:
		uint32_t m_curTex = 0;
		CallbackHandle m_cbReloadCommandBuffer;
		void InitializeCommandBuffer();
		struct Exposure
		{
			Exposure();
			std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupAverageColorTexture = nullptr;
			std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupAverageColorBuffer = nullptr;
			Vector3 averageColor;
			std::shared_ptr<prosper::Buffer> avgColorBuffer = nullptr;
			double lastExposureUpdate;
			bool Initialize(prosper::Texture &texture);
			const Vector3 &UpdateColor();
		private:
			util::WeakHandle<prosper::Shader> m_shaderCalcColor = {};
			std::weak_ptr<prosper::Texture> m_exposureColorSource = {};
			std::shared_ptr<prosper::PrimaryCommandBuffer> m_calcImgColorCmdBuffer = nullptr;
			std::shared_ptr<prosper::Fence> m_calcImgColorFence = nullptr;
			bool m_bWaitingForResult = false;
			uint32_t m_cmdBufferQueueFamilyIndex = 0u;
		} m_exposure;
		Bool m_bMipmapInitialized;
	};
	struct DLLCLIENT GlowInfo
	{
		GlowInfo();
		~GlowInfo();
		GlowInfo(const GlowInfo&)=delete;
		GlowInfo &operator=(const GlowInfo&)=delete;
		bool Initialize(uint32_t width,uint32_t height,const HDRInfo &hdrInfo);
		//Vulkan::DescriptorSet descSetAdditive; // prosper TODO
		util::WeakHandle<prosper::Shader> shader = {};
		std::vector<pragma::CParticleSystemComponent*> tmpBloomParticles;
		bool bGlowScheduled = false; // Glow meshes scheduled for this frame? (=tmpGlowMeshes isn't empty)
		std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;
		std::shared_ptr<prosper::BlurSet> blurSet = nullptr;
		//Shader::GaussianBlur::BlurRenderTarget renderTarget; // prosper TODO
		//Vulkan::RenderPass renderPass; // prosper TODO
		//Vulkan::Framebuffer framebuffer; // prosper TODO
		//Vulkan::CommandBuffer cmdBufferBlur;
		//std::function<void(Vulkan::CommandBufferObject*)> cmdBufferBlur; // prosper TODO
	private:
		CallbackHandle m_cbReloadCommandBuffer;
	};
	struct DLLCLIENT RenderInfo
	{
		RenderInfo()=default;
		RenderInfo(const RenderInfo&)=delete;
		RenderInfo &operator=(const RenderInfo&)=delete;
		std::unordered_map<BaseEntity*,bool> processed = {};
		std::vector<std::unique_ptr<ShaderMeshContainer>> containers = {};
		std::vector<std::unique_ptr<RenderSystem::MaterialMeshContainer>> glowMeshes = {};
		std::vector<std::unique_ptr<RenderSystem::TranslucentMesh>> translucentMeshes = {};
	};
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
		CreateInfo(uint32_t width,uint32_t height,float fov,float fovView,float nearZ,float farZ);
		uint32_t width;
		uint32_t height;
		float fov;
		float fovView;
		float nearZ;
		float farZ;
		Anvil::SampleCountFlagBits sampleCount;
	};

	struct DLLCLIENT LightMapInfo
	{
		std::shared_ptr<prosper::Texture> lightMapTexture = nullptr;
	};

	static std::shared_ptr<Scene> Create(const CreateInfo &createInfo);
	enum class PrepassMode : uint32_t
	{
		NoPrepass = 0,
		DepthOnly,
		Extended
	};
	//static void ClearLightCache();
	~Scene();
	Scene(Scene&)=delete;
	Scene &operator=(const Scene&)=delete;
	//const Vulkan::DescriptorSet *GetCSMShadowDescriptorSet(uint32_t layer,uint32_t swapIdx=0); // prosper TODO
	//const Vulkan::Buffer *GetCSMShadowBuffer(uint32_t layer,uint32_t swapIdx=0); // prosper TODO
	float GetFOV();
	float GetViewFOV();
	float GetAspectRatio();
	float GetZNear();
	float GetZFar();
	void BeginDraw();
	const std::shared_ptr<Camera> &GetCamera() const;
	std::shared_ptr<Camera> camera;

	void SetPrepassMode(PrepassMode mode);
	PrepassMode GetPrepassMode() const;

	void SetLightMap(const std::shared_ptr<prosper::Texture> &lightMapTexture);
	const std::shared_ptr<prosper::Texture> &GetLightMap() const;

	void SetShaderOverride(const std::string &srcShader,const std::string &shaderOverride);
	pragma::ShaderTextured3D *GetShaderOverride(pragma::ShaderTextured3D *srcShader);
	void ClearShaderOverride(const std::string &srcShader);

	const std::vector<Plane> &GetFrustumPlanes() const;
	const std::vector<Plane> &GetClippedFrustumPlanes() const;

	void InitializeRenderTarget();

	// SSAO
	bool IsSSAOEnabled() const;
	void SetSSAOEnabled(bool b);

	// Culled objects
	//const std::vector<CLightBase*> &GetCulledLights() const;
	//std::vector<CLightBase*> &GetCulledLights();
	void SetLights(const std::vector<pragma::CLightComponent*> &lights);
	void SetLights(const std::shared_ptr<LightListInfo> &lights);
	void AddLight(pragma::CLightComponent *light);
	void RemoveLight(pragma::CLightComponent *light);
	const std::shared_ptr<LightListInfo> &GetLightSourceListInfo() const;
	const std::vector<util::WeakHandle<pragma::CLightComponent>> &GetLightSources() const;
	std::vector<util::WeakHandle<pragma::CLightComponent>> &GetLightSources();
	bool HasLightSource(pragma::CLightComponent &lightSource) const;
	//void CullLightSources();

	void SetEntities(const std::vector<CBaseEntity*> &ents);
	void SetEntities(const std::shared_ptr<EntityListInfo> &ents);
	void AddEntity(CBaseEntity &ent);
	void RemoveEntity(CBaseEntity &ent);
	const std::shared_ptr<EntityListInfo> &GetEntityListInfo() const;
	const std::vector<EntityHandle> &GetEntities() const;
	std::vector<EntityHandle> &GetEntities();
	bool HasEntity(CBaseEntity &ent) const;

	const std::vector<pragma::OcclusionMeshInfo> &GetCulledMeshes() const;
	std::vector<pragma::OcclusionMeshInfo> &GetCulledMeshes();
	const std::vector<pragma::CParticleSystemComponent*> &GetCulledParticles() const;
	std::vector<pragma::CParticleSystemComponent*> &GetCulledParticles();
	//const Vulkan::DescriptorSet &GetBloomGlowDescriptorSet() const; // prosper TODO

	void PrepareRendering(RenderMode mode,bool bUpdateTranslucentMeshes=false,bool bUpdateGlowMeshes=false);

	void UpdateLightDescriptorSets(const std::vector<pragma::CLightComponent*> &lightSources);
	Anvil::DescriptorSet *GetCSMDescriptorSet() const;
	//Anvil::DescriptorSet *GetLightSourceDescriptorSet() const;

	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	void Resize(uint32_t width,uint32_t height);

	Anvil::DescriptorSet *GetDepthDescriptorSet() const;
	/*Vulkan::Texture &GetDepthTexture();
	const Vulkan::RenderTarget &GetRenderTarget() const;
	const Vulkan::Texture &GetGlowTexture() const;
	const Vulkan::DescriptorSet &GetRenderDepthDescriptorSet() const;
	const Vulkan::DescriptorSet &GetScreenDescriptorSet() const;
	const Vulkan::Texture &GetRenderDepthBuffer() const;
	const Vulkan::Texture &GetRenderTexture() const;
	const Vulkan::Texture &GetBloomTexture() const;*/ // prosper TODO
	void ReloadRenderTarget();
	Float GetHDRExposure() const;
	Float GetMaxHDRExposure() const;
	void SetMaxHDRExposure(Float exposure);
	Scene::HDRInfo &GetHDRInfo();
	Scene::GlowInfo &GetGlowInfo();
	SSAOInfo &GetSSAOInfo();
	pragma::rendering::Prepass &GetPrepass();
	pragma::rendering::ForwardPlusInstance &GetForwardPlusInstance();
	RenderInfo *GetRenderInfo(RenderMode mode) const;
	Anvil::SampleCountFlagBits GetSampleCount() const;
	bool IsMultiSampled() const;

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

	void UpdateTileSize();

	bool BeginRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,prosper::RenderPass *customRenderPass=nullptr);
	bool EndRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);
	bool ResolveRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);
	/*void BeginRenderPass(Vulkan::CommandBufferObject *drawCmd,const Color *clearColor=nullptr);
	void EndRenderPass(Vulkan::CommandBufferObject *drawCmd);
	void Present(Vulkan::CommandBufferObject *drawCmd,const Vulkan::RenderPassObject *renderPass,const Vulkan::FramebufferObject *framebuffer);
	void Present(Vulkan::CommandBufferObject *drawCmd);

	Vulkan::Texture &ResolveRenderTexture(Vulkan::CommandBufferObject *cmdBuffer);
	Vulkan::Texture &ResolveBloomTexture(Vulkan::CommandBufferObject *cmdBuffer);
	Vulkan::Texture &ResolveDepthTexture(Vulkan::CommandBufferObject *cmdBuffer);*/ // prosper TODO

	OcclusionOctree<CBaseEntity*> &GetOcclusionOctree();
	const OcclusionOctree<CBaseEntity*> &GetOcclusionOctree() const;

	const pragma::OcclusionCullingHandler &GetOcclusionCullingHandler() const;
	pragma::OcclusionCullingHandler &GetOcclusionCullingHandler();
	void SetOcclusionCullingHandler(const std::shared_ptr<pragma::OcclusionCullingHandler> &handler);
	void ReloadOcclusionCullingHandler();

	void LinkLightSources(Scene &other);
	void LinkEntities(Scene &other);
	void LinkWorldEnvironment(Scene &other);

	pragma::ShaderPrepassBase &GetPrepassShader() const;
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

	// SSAO
	bool m_bSSAOEnabled = false;

	// Render Target
	uint32_t m_width;
	uint32_t m_height;
	bool m_bDepthResolved;
	bool m_bBloomResolved;
	bool m_bRenderResolved;
	Anvil::SampleCountFlagBits m_sampleCount = Anvil::SampleCountFlagBits::_1_BIT;
	std::shared_ptr<pragma::OcclusionCullingHandler> m_occlusionCullingHandler = nullptr;
	std::shared_ptr<OcclusionOctree<CBaseEntity*>> m_occlusionOctree = nullptr;

	LightMapInfo m_lightMapInfo = {};

	//Vulkan::RenderTarget m_renderTarget = nullptr; // prosper TODO
	//Vulkan::DescriptorSet m_descSetScreen = nullptr; // prosper TODO

	//Vulkan::DescriptorSet m_descSetBloomGlow = nullptr; // prosper TODO

	/*
	Vulkan::DescriptorSetLayout m_descSetLayoutCamGraphics = nullptr;
	Vulkan::DescriptorSetLayout m_descSetLayoutCamCompute = nullptr;
	Vulkan::SwapDescriptorBuffer m_swapDescBufferCamGraphics = nullptr;
	Vulkan::SwapDescriptorBuffer m_swapDescBufferCamCompute = nullptr;
	Vulkan::SwapDescriptorBuffer m_swapDescBufferViewCam = nullptr;
	*/ // prosper TODO
	std::shared_ptr<prosper::DescriptorSetGroup> m_camDescSetGroupGraphics = nullptr;
	std::shared_ptr<prosper::DescriptorSetGroup> m_camDescSetGroupCompute = nullptr;
	std::shared_ptr<prosper::DescriptorSetGroup> m_camViewDescSetGroup = nullptr;

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

	std::unordered_map<size_t,::util::WeakHandle<prosper::Shader>> m_shaderOverrides;
	mutable ::util::WeakHandle<prosper::Shader> m_whShaderWireframe = {};
	bool m_bValid = false;

	// HDR
	HDRInfo m_hdrInfo;
	GlowInfo m_glowInfo;
	bool m_bPrepassEnabled = true;

	// Frustum planes (Required for culling)
	std::vector<Plane> m_frustumPlanes = {};
	std::vector<Plane> m_clippedFrustumPlanes = {};
	void UpdateFrustumPlanes();

	mutable std::unordered_map<RenderMode,std::shared_ptr<RenderInfo>> m_renderInfo;

	std::shared_ptr<LightListInfo> m_lightSources = nullptr;
	std::shared_ptr<EntityListInfo> m_entityList = nullptr;

	// Culled objects
	std::vector<pragma::OcclusionMeshInfo> m_culledMeshes;
	std::vector<pragma::CParticleSystemComponent*> m_culledParticles;

	void UpdateCameraBuffer(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,bool bView=false);
	void UpdateRenderSettings();

	// Light Sources
	std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupCSM;
	void InitializeLightDescriptorSets();
	void InitializeRenderSettingsBuffer();
	void InitializeCameraBuffer();
	void InitializeFogBuffer();
	void InitializeDescriptorSetLayouts();
	void InitializeSwapDescriptorBuffers();
};
REGISTER_BASIC_BITWISE_OPERATORS(Scene::FRenderSetting);
#pragma warning(pop)

#endif
