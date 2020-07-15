/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __RASERIZATION_RENDERER_HPP__
#define __RASERIZATION_RENDERER_HPP__

#include "pragma/rendering/renderers/base_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/glow_data.hpp"
#include "pragma/rendering/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/rendering/render_mesh_collection_handler.hpp"
#include <pragma/math/plane.h>
#include <sharedutils/util_weak_handle.hpp>
#include <string>
#include <vector>

namespace prosper
{
	class Texture;
	class IDescriptorSetGroup;
	class IPrimaryCommandBuffer;
	class IDescriptorSet;
};
namespace pragma
{
	class ShaderTextured3DBase;
	class ShaderPrepassBase;
	class CLightComponent;
	class CParticleSystemComponent;
	struct OcclusionMeshInfo;
	class OcclusionCullingHandler;
	class CLightDirectionalComponent;
	class CSkyCameraComponent;
};
namespace pragma::rendering
{
	struct DLLCLIENT RendererData
	{
		enum class Flags : uint32_t
		{
			None = 0u,
			SSAOEnabled = 1u
		};
		void SetResolution(uint32_t w,uint32_t h)
		{
			vpResolution = w<<16 | (static_cast<uint16_t>(h));
		}
		Flags flags = Flags::None;
		uint32_t vpResolution = 0;
		uint32_t tileInfo; // First 16 bits = number of tiles (x-axis), second 16 bits = tile size
	};

	class Prepass;
	class ForwardPlusInstance;
	struct CulledMeshData;
	class DLLCLIENT RasterizationRenderer
		: public BaseRenderer
	{
	public:
		enum class PrepassMode : uint32_t
		{
			NoPrepass = 0,
			DepthOnly,
			Extended
		};

		enum class StateFlags : uint32_t
		{
			None = 0u,
			DepthResolved = 1u,
			BloomResolved = DepthResolved<<1u,
			RenderResolved = BloomResolved<<1u,

			SSAOEnabled = RenderResolved<<1u,
			PrepassEnabled = SSAOEnabled<<1u
		};

		enum class Stage : uint8_t
		{
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

			Final
		};

		struct DLLCLIENT LightMapInfo
		{
			std::shared_ptr<prosper::Texture> lightMapTexture = nullptr;
		};
		virtual ~RasterizationRenderer() override;

		virtual void EndRendering() override;
		virtual void UpdateRenderSettings() override;
		virtual void UpdateCameraData(pragma::CameraData &cameraData) override;
		virtual bool ReloadRenderTarget(uint32_t width,uint32_t height) override;
		using BaseRenderer::GetSceneTexture;
		using BaseRenderer::GetPresentationTexture;
		using BaseRenderer::GetHDRPresentationTexture;
		virtual prosper::Texture *GetSceneTexture() override;
		virtual prosper::Texture *GetPresentationTexture() override;
		virtual prosper::Texture *GetHDRPresentationTexture() override;
		virtual bool IsRasterizationRenderer() const override;

		void SetPrepassMode(PrepassMode mode);
		PrepassMode GetPrepassMode() const;

		void SetLightMap(const std::shared_ptr<prosper::Texture> &lightMapTexture);
		const std::shared_ptr<prosper::Texture> &GetLightMap() const;

		void SetShaderOverride(const std::string &srcShader,const std::string &shaderOverride);
		pragma::ShaderTextured3DBase *GetShaderOverride(pragma::ShaderTextured3DBase *srcShader);
		void ClearShaderOverride(const std::string &srcShader);

		const std::vector<Plane> &GetFrustumPlanes() const;
		const std::vector<Plane> &GetClippedFrustumPlanes() const;

		// SSAO
		bool IsSSAOEnabled() const;
		void SetSSAOEnabled(bool b);

		// Culled objects
		const std::vector<pragma::OcclusionMeshInfo> &GetCulledMeshes() const;
		std::vector<pragma::OcclusionMeshInfo> &GetCulledMeshes();
		const std::vector<pragma::CParticleSystemComponent*> &GetCulledParticles() const;
		std::vector<pragma::CParticleSystemComponent*> &GetCulledParticles();

		Float GetHDRExposure() const;
		Float GetMaxHDRExposure() const;
		void SetMaxHDRExposure(Float exposure);
		HDRData &GetHDRInfo();
		GlowData &GetGlowInfo();
		SSAOInfo &GetSSAOInfo();

		prosper::IDescriptorSet *GetDepthDescriptorSet() const;
		void SetFogOverride(const std::shared_ptr<prosper::IDescriptorSetGroup> &descSetGroup);
		prosper::IDescriptorSet *GetRendererDescriptorSet() const;

		pragma::rendering::Prepass &GetPrepass();
		const pragma::rendering::ForwardPlusInstance &GetForwardPlusInstance() const;
		pragma::rendering::ForwardPlusInstance &GetForwardPlusInstance();
		CulledMeshData *GetRenderInfo(RenderMode mode) const;
		prosper::SampleCountFlags GetSampleCount() const;
		bool IsMultiSampled() const;

		bool BeginRenderPass(const util::DrawSceneInfo &drawSceneInfo,prosper::IRenderPass *customRenderPass=nullptr);
		bool EndRenderPass(const util::DrawSceneInfo &drawSceneInfo);
		bool ResolveRenderPass(const util::DrawSceneInfo &drawSceneInfo);
		void PrepareRendering(RenderMode mode,FRender renderFlags,bool bUpdateTranslucentMeshes=false,bool bUpdateGlowMeshes=false);

		const pragma::OcclusionCullingHandler &GetOcclusionCullingHandler() const;
		pragma::OcclusionCullingHandler &GetOcclusionCullingHandler();
		void SetOcclusionCullingHandler(const std::shared_ptr<pragma::OcclusionCullingHandler> &handler);
		void ReloadOcclusionCullingHandler();

		pragma::ShaderPrepassBase &GetPrepassShader() const;

		// Render
		void RenderParticleSystems(const util::DrawSceneInfo &drawSceneInfo,std::vector<pragma::CParticleSystemComponent*> &particles,RenderMode renderMode,Bool bloom=false,std::vector<pragma::CParticleSystemComponent*> *bloomParticles=nullptr);

		// Renders all meshes from m_glowInfo.tmpGlowMeshes, and clears the container when done
		void RenderGlowMeshes(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,RenderMode renderMode);

		// If this flag is set, the prepass depth buffer will be blitted into a sampleable buffer
		// before rendering, which can then be used as shader sampler input. This flag will be reset once
		// rendering has finished.
		void SetFrameDepthBufferSamplingRequired();

		RenderMeshCollectionHandler &GetRenderMeshCollectionHandler();
		const RenderMeshCollectionHandler &GetRenderMeshCollectionHandler() const;

		prosper::IDescriptorSet *GetLightSourceDescriptorSet() const;
		prosper::IDescriptorSet *GetLightSourceDescriptorSetCompute() const;

		prosper::Shader *GetWireframeShader();
		virtual bool RenderScene(const util::DrawSceneInfo &drawSceneInfo) override;
		void UpdateRendererBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		void UpdateCSMDescriptorSet(pragma::CLightDirectionalComponent &lightSource);
	private:
		friend BaseRenderer;
		RasterizationRenderer(Scene &scene);
		void InitializeLightDescriptorSets();

		void RenderGameScene(const util::DrawSceneInfo &drawSceneInfo);

		void PerformOcclusionCulling();
		void CollectRenderObjects(FRender renderFlags);
		void RenderPrepass(const util::DrawSceneInfo &drawSceneInfo);
		void RenderSSAO(const util::DrawSceneInfo &drawSceneInfo);
		void CullLightSources(const util::DrawSceneInfo &drawSceneInfo);
		void RenderLightingPass(const util::DrawSceneInfo &drawSceneInfo);
		void RenderGlowObjects(const util::DrawSceneInfo &drawSceneInfo);
		void RenderBloom(const util::DrawSceneInfo &drawSceneInfo);
		void RenderToneMapping(const util::DrawSceneInfo &drawSceneInfo,prosper::IDescriptorSet &descSetHdrResolve);
		void RenderFXAA(const util::DrawSceneInfo &drawSceneInfo);

		virtual bool Initialize() override;
		virtual void BeginRendering(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;

		void RenderSceneFog(const util::DrawSceneInfo &drawSceneInfo);

		StateFlags m_stateFlags = StateFlags::PrepassEnabled;

		prosper::SampleCountFlags m_sampleCount = prosper::SampleCountFlags::e1Bit;
		std::shared_ptr<pragma::OcclusionCullingHandler> m_occlusionCullingHandler = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupFogOverride = nullptr;

		LightMapInfo m_lightMapInfo = {};
		bool m_bFrameDepthBufferSamplingRequired = false;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dsgLights;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dsgLightsCompute;

		// HDR
		HDRData m_hdrInfo;
		GlowData m_glowInfo;

		// Frustum planes (Required for culling)
		std::vector<Plane> m_frustumPlanes = {};
		std::vector<Plane> m_clippedFrustumPlanes = {};
		void UpdateFrustumPlanes();

		RendererData m_rendererData {};
		std::shared_ptr<prosper::IBuffer> m_rendererBuffer = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupRenderer = nullptr;

		// 3D sky cameras used for the current rendering pass
		std::vector<util::WeakHandle<pragma::CSkyCameraComponent>> m_3dSkyCameras = {};

		RenderMeshCollectionHandler m_renderMeshCollectionHandler = {};

		std::unordered_map<size_t,::util::WeakHandle<prosper::Shader>> m_shaderOverrides;
		mutable ::util::WeakHandle<prosper::Shader> m_whShaderWireframe = {};
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::RasterizationRenderer::StateFlags)
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::RendererData::Flags)

#endif
