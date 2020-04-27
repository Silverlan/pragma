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
#include <misc/types.h>
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
		virtual void UpdateRenderSettings(pragma::RenderSettings &renderSettings) override;
		virtual void UpdateCameraData(pragma::CameraData &cameraData) override;
		virtual bool ReloadRenderTarget() override;
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
		//const Vulkan::DescriptorSet &GetBloomGlowDescriptorSet() const; // prosper TODO
		prosper::IDescriptorSet *GetCSMDescriptorSet() const;
		//prosper::IDescriptorSet *GetLightSourceDescriptorSet() const;

		Float GetHDRExposure() const;
		Float GetMaxHDRExposure() const;
		void SetMaxHDRExposure(Float exposure);
		HDRData &GetHDRInfo();
		GlowData &GetGlowInfo();
		SSAOInfo &GetSSAOInfo();

		prosper::IDescriptorSet *GetDepthDescriptorSet() const;
		void UpdateCSMDescriptorSet(pragma::CLightDirectionalComponent &lightSource);
		void SetFogOverride(const std::shared_ptr<prosper::IDescriptorSetGroup> &descSetGroup);

		pragma::rendering::Prepass &GetPrepass();
		const pragma::rendering::ForwardPlusInstance &GetForwardPlusInstance() const;
		pragma::rendering::ForwardPlusInstance &GetForwardPlusInstance();
		CulledMeshData *GetRenderInfo(RenderMode mode) const;
		prosper::SampleCountFlags GetSampleCount() const;
		bool IsMultiSampled() const;

		bool BeginRenderPass(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,prosper::IRenderPass *customRenderPass=nullptr);
		bool EndRenderPass(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		bool ResolveRenderPass(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		void PrepareRendering(RenderMode mode,FRender renderFlags,bool bUpdateTranslucentMeshes=false,bool bUpdateGlowMeshes=false);

		const pragma::OcclusionCullingHandler &GetOcclusionCullingHandler() const;
		pragma::OcclusionCullingHandler &GetOcclusionCullingHandler();
		void SetOcclusionCullingHandler(const std::shared_ptr<pragma::OcclusionCullingHandler> &handler);
		void ReloadOcclusionCullingHandler();

		pragma::ShaderPrepassBase &GetPrepassShader() const;

		// Render
		void RenderParticleSystems(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,std::vector<pragma::CParticleSystemComponent*> &particles,RenderMode renderMode,Bool bloom=false,std::vector<pragma::CParticleSystemComponent*> *bloomParticles=nullptr);

		// Renders all meshes from m_glowInfo.tmpGlowMeshes, and clears the container when done
		void RenderGlowMeshes(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,RenderMode renderMode);

		// If this flag is set, the prepass depth buffer will be blitted into a sampleable buffer
		// before rendering, which can then be used as shader sampler input. This flag will be reset once
		// rendering has finished.
		void SetFrameDepthBufferSamplingRequired();

		RenderMeshCollectionHandler &GetRenderMeshCollectionHandler();
		const RenderMeshCollectionHandler &GetRenderMeshCollectionHandler() const;

		prosper::Shader *GetWireframeShader();
		virtual bool RenderScene(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,FRender renderFlags=FRender::All) override;
	private:
		friend BaseRenderer;
		RasterizationRenderer(Scene &scene);

		void RenderGameScene(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,FRender renderFlags);

		void PerformOcclusionCulling();
		void CollectRenderObjects(FRender renderFlags);
		void RenderPrepass(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,FRender renderFlags);
		void RenderSSAO(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		void CullLightSources(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		void RenderLightingPass(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,FRender renderFlags);
		void RenderGlowObjects(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		void RenderBloom(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		void RenderToneMapping(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,prosper::IDescriptorSet &descSetHdrResolve);
		void RenderFXAA(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);

		void InitializeLightDescriptorSets();
		virtual bool Initialize() override;
		virtual void BeginRendering(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;

		void RenderSceneFog(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);

		StateFlags m_stateFlags = StateFlags::PrepassEnabled;

		prosper::SampleCountFlags m_sampleCount = prosper::SampleCountFlags::e1Bit;
		std::shared_ptr<pragma::OcclusionCullingHandler> m_occlusionCullingHandler = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupFogOverride = nullptr;

		LightMapInfo m_lightMapInfo = {};
		bool m_bFrameDepthBufferSamplingRequired = false;

		// HDR
		HDRData m_hdrInfo;
		GlowData m_glowInfo;

		// Frustum planes (Required for culling)
		std::vector<Plane> m_frustumPlanes = {};
		std::vector<Plane> m_clippedFrustumPlanes = {};
		void UpdateFrustumPlanes();

		// 3D sky cameras used for the current rendering pass
		std::vector<util::WeakHandle<pragma::CSkyCameraComponent>> m_3dSkyCameras = {};

		RenderMeshCollectionHandler m_renderMeshCollectionHandler = {};
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupCSM;

		std::unordered_map<size_t,::util::WeakHandle<prosper::Shader>> m_shaderOverrides;
		mutable ::util::WeakHandle<prosper::Shader> m_whShaderWireframe = {};
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::RasterizationRenderer::StateFlags)

#endif
