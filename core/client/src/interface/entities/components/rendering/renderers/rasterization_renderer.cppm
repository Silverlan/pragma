// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

#define DEBUG_RENDER_PERFORMANCE_TEST_ENABLED 0

export module pragma.client:entities.components.rasterization_renderer;

export import :entities.components.light_map;
export import :rendering.draw_scene_info;
export import :rendering.forward_plus;
export import :rendering.prepass;
export import :rendering.render_processor;
export import :rendering.ssao;

export namespace pragma {
	class ShaderGameWorldLightingPass;
	class CRasterizationRendererComponent;
	namespace ecs {
		class CParticleSystemComponent;
	}
};
export namespace pragma::rendering {
	class DLLCLIENT HDRData {
	  public:
		HDRData(CRasterizationRendererComponent &rasterizer);
		~HDRData();
		void UpdateExposure();
		bool Initialize(uint32_t width, uint32_t height, prosper::SampleCountFlags sampleCount, bool bEnableSSAO);
		bool InitializeDescriptorSets();
		prosper::RenderTarget &GetRenderTarget(const util::DrawSceneInfo &drawSceneInfo);

		void SwapIOTextures();

		bool BeginRenderPass(const util::DrawSceneInfo &drawSceneInfo, prosper::IRenderPass *customRenderPass = nullptr, bool secondaryCommandBuffers = false);
		bool EndRenderPass(const util::DrawSceneInfo &drawSceneInfo);
		bool ResolveRenderPass(const util::DrawSceneInfo &drawSceneInfo);
		void ReloadPresentationRenderTarget(uint32_t width, uint32_t height, prosper::SampleCountFlags sampleCount);
		bool ReloadBloomRenderTarget(uint32_t width);

		void ResetIOTextureIndex();
		bool BlitStagingRenderTargetToMainRenderTarget(const util::DrawSceneInfo &drawSceneInfo, prosper::ImageLayout srcHdrLayout = prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout dstHdrLayout = prosper::ImageLayout::ColorAttachmentOptimal);
		bool BlitMainDepthBufferToSamplableDepthBuffer(const util::DrawSceneInfo &drawSceneInfo, std::function<void(prosper::ICommandBuffer &)> &fTransitionSampleImgToTransferDst);

		SSAOInfo ssaoInfo;
		pragma::rendering::Prepass prepass;
		pragma::rendering::ForwardPlusInstance forwardPlusInstance;

		// This is the render target for the lighting pass, containing the
		// 1) color image (HDR)
		// 2) bloom color image (HDR), containing all bright colors
		// 3) depth image
		std::shared_ptr<prosper::RenderTarget> sceneRenderTarget = nullptr;
		// Bound to HDR color image; Used for HDR post-processing
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgHDRPostProcessing = nullptr;

		// Contains the bright colors of the scene, as output by the lighting pass
		std::shared_ptr<prosper::Texture> bloomTexture = nullptr;

		// Contains the blurred bright areas, is overlayed over the scene in post-processing
		std::shared_ptr<prosper::RenderTarget> bloomBlurRenderTarget = nullptr;
		std::shared_ptr<prosper::BlurSet> bloomBlurSet = nullptr;

		// Bound to HDR color image and HDR blurred bloom color image, used for tonemapping and
		// applying bloom effect
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgBloomTonemapping = nullptr;

		// Render target for post-processing after the lighting pass with HDR colors
		std::shared_ptr<prosper::RenderTarget> hdrPostProcessingRenderTarget = nullptr;

		// Render target containing image after tonemapping
		std::shared_ptr<prosper::RenderTarget> toneMappedRenderTarget = nullptr;
		// Bound to tonemapped (LDR) color image
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgTonemappedPostProcessing = nullptr;

		// Render target for post-processing after tonemapping
		std::shared_ptr<prosper::RenderTarget> toneMappedPostProcessingRenderTarget = nullptr;
		// Bound to tonemapped post-processing image
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgToneMappedPostProcessing = nullptr;

		// Bound to depth image of lighting stage; Used for particle effects
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgSceneDepth = nullptr;
		// Bound to post-scene depth image; Used for post-processing (e.g. fog)
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgDepthPostProcessing = nullptr;

		// Render pass used to restart scene pass after particle pass
		std::shared_ptr<prosper::IRenderPass> rpPostParticle = nullptr;

		// Render target used for particle render pass
		std::shared_ptr<prosper::RenderTarget> rtParticle = nullptr;

		float exposure = 1.f;
		float max_exposure = 1, f;
		std::array<float, 3> luminescence = {0.f, 0.f, 0.f};
	  private:
		static prosper::util::SamplerCreateInfo GetSamplerCreateInfo();
		uint32_t m_curTex = 0;
		CallbackHandle m_cbReloadCommandBuffer;
		struct Exposure {
			Exposure();
			std::shared_ptr<prosper::IDescriptorSetGroup> descSetGroupAverageColorTexture = nullptr;
			std::shared_ptr<prosper::IDescriptorSetGroup> descSetGroupAverageColorBuffer = nullptr;
			Vector3 averageColor;
			std::shared_ptr<prosper::IBuffer> avgColorBuffer = nullptr;
			double lastExposureUpdate;
			bool Initialize(prosper::Texture &texture);
			const Vector3 &UpdateColor();
		  private:
			util::WeakHandle<prosper::Shader> m_shaderCalcColor = {};
			std::weak_ptr<prosper::Texture> m_exposureColorSource = {};
			std::shared_ptr<prosper::IPrimaryCommandBuffer> m_calcImgColorCmdBuffer = nullptr;
			std::shared_ptr<prosper::IFence> m_calcImgColorFence = nullptr;
			bool m_bWaitingForResult = false;
			uint32_t m_cmdBufferQueueFamilyIndex = 0u;
		} m_exposure;
		Bool m_bMipmapInitialized;
	};
};

export namespace pragma {
	struct DLLCLIENT RendererData {
		enum class Flags : uint32_t { None = 0u, SSAOEnabled = 1u };
		void SetResolution(uint32_t w, uint32_t h) { vpResolution = w << 16 | (static_cast<uint16_t>(h)); }
		Flags flags = Flags::None;
		uint32_t vpResolution = 0;
		uint32_t tileInfo; // First 16 bits = number of tiles (x-axis), second 16 bits = tile size
		float lightmapExposurePow = 0.f;
		float bloomThreshold = 1.f;
	};

	class DLLCLIENT CRasterizationRendererComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_RECORD_PREPASS;
		static ComponentEventId EVENT_ON_RECORD_LIGHTING_PASS;
		static ComponentEventId EVENT_PRE_EXECUTE_PREPASS;
		static ComponentEventId EVENT_POST_EXECUTE_PREPASS;
		static ComponentEventId EVENT_PRE_EXECUTE_LIGHTING_PASS;
		static ComponentEventId EVENT_POST_EXECUTE_LIGHTING_PASS;
		static ComponentEventId EVENT_PRE_PREPASS;
		static ComponentEventId EVENT_POST_PREPASS;
		static ComponentEventId EVENT_PRE_LIGHTING_PASS;
		static ComponentEventId EVENT_POST_LIGHTING_PASS;
		static ComponentEventId EVENT_UPDATE_RENDER_BUFFERS;

		static ComponentEventId EVENT_MT_BEGIN_RECORD_SKYBOX;
		static ComponentEventId EVENT_MT_END_RECORD_SKYBOX;
		static ComponentEventId EVENT_MT_BEGIN_RECORD_WORLD;
		static ComponentEventId EVENT_MT_END_RECORD_WORLD;
		static ComponentEventId EVENT_MT_BEGIN_RECORD_PARTICLES;
		static ComponentEventId EVENT_MT_END_RECORD_PARTICLES;
		static ComponentEventId EVENT_MT_BEGIN_RECORD_DEBUG;
		static ComponentEventId EVENT_MT_END_RECORD_DEBUG;
		static ComponentEventId EVENT_MT_BEGIN_RECORD_WATER;
		static ComponentEventId EVENT_MT_END_RECORD_WATER;
		static ComponentEventId EVENT_MT_BEGIN_RECORD_VIEW;
		static ComponentEventId EVENT_MT_END_RECORD_VIEW;
		static ComponentEventId EVENT_MT_BEGIN_RECORD_PREPASS;

		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		static void UpdateLightmap(CLightMapComponent &lightMapC);
		static void UpdateLightmap();
		enum class PrepassMode : uint32_t { NoPrepass = 0, DepthOnly, Extended };

		enum class StateFlags : uint32_t {
			None = 0u,
			DepthResolved = 1u,
			BloomResolved = DepthResolved << 1u,
			RenderResolved = BloomResolved << 1u,

			SSAOEnabled = RenderResolved << 1u,
			PrepassEnabled = SSAOEnabled << 1u,

			InitialRender = PrepassEnabled << 1u,
		};

		enum class Stage : uint8_t {
			Initial = 0,
			OcclusionCulling,
			CollectRenderObjects,
			Prepass,
			SSAOPass,
			LightCullingPass,
			LightingPass,
			PostProcessingPass,
			PPFog,
			PPGlow,
			PPBloom,
			PPToneMapping,
			PPFXAA,
			PPDoF,

			Final
		};

		struct DLLCLIENT LightMapInfo {
			~LightMapInfo()
			{
				if(cbExposure.IsValid())
					cbExposure.Remove();
			}
			std::shared_ptr<prosper::Texture> lightMapTexture = nullptr;
			std::shared_ptr<prosper::Texture> lightMapIndirectTexture = nullptr;
			std::shared_ptr<prosper::Texture> lightMapDominantDirectionTexture = nullptr;
			ComponentHandle<pragma::CLightMapComponent> lightMapComponent {};
			CallbackHandle cbExposure {};
		};

		CRasterizationRendererComponent(pragma::ecs::BaseEntity &ent);
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void Initialize() override;

		friend SceneRenderDesc;
		virtual ~CRasterizationRendererComponent() override;

		void ReloadPresentationRenderTarget();

		void SetPrepassMode(PrepassMode mode);
		PrepassMode GetPrepassMode() const;

		void SetLightMap(pragma::CLightMapComponent &lightMapC);
		const ComponentHandle<pragma::CLightMapComponent> &GetLightMap() const;
		bool HasIndirectLightmap() const;
		bool HasDirectionalLightmap() const;

		void SetShaderOverride(const std::string &srcShader, const std::string &shaderOverride);
		pragma::ShaderGameWorldLightingPass *GetShaderOverride(pragma::ShaderGameWorldLightingPass *srcShader) const;
		void ClearShaderOverride(const std::string &srcShader);

		const std::vector<umath::Plane> &GetFrustumPlanes() const;
		const std::vector<umath::Plane> &GetClippedFrustumPlanes() const;

		// SSAO
		bool IsSSAOEnabled() const;
		void SetSSAOEnabled(bool b);

		Float GetHDRExposure() const;
		Float GetMaxHDRExposure() const;
		void SetMaxHDRExposure(Float exposure);
		const rendering::HDRData &GetHDRInfo() const;
		rendering::HDRData &GetHDRInfo();
		// GlowData &GetGlowInfo();
		SSAOInfo &GetSSAOInfo();

		void SetBloomThreshold(float threshold);
		float GetBloomThreshold() const;

		prosper::IDescriptorSet *GetDepthDescriptorSet() const;
		void SetFogOverride(const std::shared_ptr<prosper::IDescriptorSetGroup> &descSetGroup);
		const std::shared_ptr<prosper::IDescriptorSetGroup> &GetFogOverride() const;
		prosper::IDescriptorSet *GetRendererDescriptorSet() const;

		pragma::rendering::Prepass &GetPrepass();
		const pragma::rendering::ForwardPlusInstance &GetForwardPlusInstance() const;
		pragma::rendering::ForwardPlusInstance &GetForwardPlusInstance();
		prosper::SampleCountFlags GetSampleCount() const;
		bool IsMultiSampled() const;

		prosper::RenderTarget *BeginRenderPass(const util::DrawSceneInfo &drawSceneInfo, prosper::IRenderPass *customRenderPass = nullptr, bool secondaryCommandBuffers = false);
		bool EndRenderPass(const util::DrawSceneInfo &drawSceneInfo);
		bool ResolveRenderPass(const util::DrawSceneInfo &drawSceneInfo);

		pragma::ShaderPrepassBase &GetPrepassShader() const;

		// Render
		void RecordRenderParticleSystems(prosper::ICommandBuffer &cmd, const util::DrawSceneInfo &drawSceneInfo, const std::vector<pragma::ecs::CParticleSystemComponent *> &particles, pragma::rendering::SceneRenderPass renderMode, bool depthPass, Bool bloom = false);

		// Renders all meshes from m_glowInfo.tmpGlowMeshes, and clears the container when done
		void RenderGlowMeshes(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, const CSceneComponent &scene, pragma::rendering::SceneRenderPass renderMode);

		// If this flag is set, the prepass depth buffer will be blitted into a sampleable buffer
		// before rendering, which can then be used as shader sampler input. This flag will be reset once
		// rendering has finished.
		void SetFrameDepthBufferSamplingRequired();

		prosper::IDescriptorSet *GetLightSourceDescriptorSetCompute() const;

		prosper::Shader *GetWireframeShader() const;
		void UpdateCSMDescriptorSet(pragma::BaseEnvLightDirectionalComponent &lightSource);

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

		template<typename TCPPM>
		TCPPM *GetRendererComponent();
		template<typename TCPPM>
		const TCPPM *GetRendererComponent() const;

		void StartPrepassRecording(const util::DrawSceneInfo &drawSceneInfo);
		void StartLightingPassRecording(const util::DrawSceneInfo &drawSceneInfo);

		void RecordLightingPass(const util::DrawSceneInfo &drawSceneInfo);
		void ExecuteLightingPass(const util::DrawSceneInfo &drawSceneInfo);

		void UpdatePrepassRenderBuffers(const util::DrawSceneInfo &drawSceneInfo);
		void UpdateLightingPassRenderBuffers(const util::DrawSceneInfo &drawSceneInfo);
		void RecordPrepass(const util::DrawSceneInfo &drawSceneInfo);
		void ExecutePrepass(const util::DrawSceneInfo &drawSceneInfo);

		const std::vector<pragma::ecs::CParticleSystemComponent *> &GetCulledParticles() const { return m_culledParticles; }

		const std::shared_ptr<prosper::ISwapCommandBufferGroup> &GetPrepassCommandBufferRecorder() const { return m_prepassCommandBufferGroup; }
		const std::shared_ptr<prosper::ISwapCommandBufferGroup> &GetShadowCommandBufferRecorder() const { return m_shadowCommandBufferGroup; }
		const std::shared_ptr<prosper::ISwapCommandBufferGroup> &GetLightingPassCommandBufferRecorder() const { return m_lightingCommandBufferGroup; }
	  private:
		void UpdateRendererBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		void UpdateRenderSettings();
		bool ReloadRenderTarget(uint32_t width, uint32_t height);
		void InitializeLightDescriptorSets();
		void RecordCommandBuffers(const util::DrawSceneInfo &drawSceneInfo);
		void Render(const util::DrawSceneInfo &drawSceneInfo);

		void EndRendering();
		void BeginRendering(const util::DrawSceneInfo &drawSceneInfo);

		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		prosper::RenderTarget *GetPrepassRenderTarget(const util::DrawSceneInfo &drawSceneInfo);
		prosper::RenderTarget *GetLightingPassRenderTarget(const util::DrawSceneInfo &drawSceneInfo);

		void RenderSSAO(const util::DrawSceneInfo &drawSceneInfo);
		void CullLightSources(const util::DrawSceneInfo &drawSceneInfo);
		void RenderShadows(const util::DrawSceneInfo &drawSceneInfo);
		void RenderGlowObjects(const util::DrawSceneInfo &drawSceneInfo);

		void RenderParticles(prosper::ICommandBuffer &cmdBuffer, const util::DrawSceneInfo &drawSceneInfo, bool depthPass, prosper::IPrimaryCommandBuffer *primCmdBuffer = nullptr);

		void InitializeCommandBufferGroups();

		std::shared_ptr<prosper::ISwapCommandBufferGroup> m_prepassCommandBufferGroup = nullptr;
		std::shared_ptr<prosper::ISwapCommandBufferGroup> m_shadowCommandBufferGroup = nullptr;
		std::shared_ptr<prosper::ISwapCommandBufferGroup> m_lightingCommandBufferGroup = nullptr;

		StateFlags m_stateFlags;
		BaseEntityComponent *m_rendererComponent = nullptr;

		prosper::SampleCountFlags m_sampleCount = prosper::SampleCountFlags::e1Bit;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupFogOverride = nullptr;

		LightMapInfo m_lightMapInfo = {};
		bool m_bFrameDepthBufferSamplingRequired = false;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dsgLightsCompute;

		std::vector<pragma::CLightComponent *> m_visLightSources;
		std::vector<ComponentHandle<pragma::CLightComponent>> m_visShadowedLights;
		std::vector<pragma::ecs::CParticleSystemComponent *> m_culledParticles;

		// HDR
		rendering::HDRData m_hdrInfo;
		// GlowData m_glowInfo;

		// Frustum planes (Required for culling)
		std::vector<umath::Plane> m_frustumPlanes = {};
		std::vector<umath::Plane> m_clippedFrustumPlanes = {};
		void UpdateFrustumPlanes(CSceneComponent &scene);

		RendererData m_rendererData {};
		std::shared_ptr<prosper::IBuffer> m_rendererBuffer = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupRenderer = nullptr;

		std::unordered_map<size_t, ::util::WeakHandle<prosper::Shader>> m_shaderOverrides;
		mutable ::util::WeakHandle<prosper::Shader> m_whShaderWireframe = {};
	};

	struct DLLCLIENT CELightingStageData : public ComponentEvent {
		CELightingStageData(pragma::rendering::LightingStageRenderProcessor &renderProcessor);
		virtual void PushArguments(lua_State *l) override;
		pragma::rendering::LightingStageRenderProcessor &renderProcessor;
	};

	struct DLLCLIENT CEPrepassStageData : public ComponentEvent {
		CEPrepassStageData(pragma::rendering::DepthStageRenderProcessor &renderProcessor, pragma::ShaderPrepassBase &shader);
		virtual void PushArguments(lua_State *l) override;
		pragma::rendering::DepthStageRenderProcessor &renderProcessor;
		pragma::ShaderPrepassBase &shader;
	};

	struct DLLCLIENT CEUpdateRenderBuffers : public ComponentEvent {
		CEUpdateRenderBuffers(const util::DrawSceneInfo &drawSceneInfo);
		virtual void PushArguments(lua_State *l) override;
		const util::DrawSceneInfo &drawSceneInfo;
	};
	using namespace umath::scoped_enum::bitwise;
};
export {
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<pragma::CRasterizationRendererComponent::StateFlags> : std::true_type {};

		template<>
		struct enable_bitwise_operators<pragma::RendererData::Flags> : std::true_type {};
	}

	class DLLCLIENT CRasterizationRenderer : public CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
