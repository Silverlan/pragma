#ifndef __RASERIZATION_RENDERER_HPP__
#define __RASERIZATION_RENDERER_HPP__

#include "pragma/rendering/renderers/base_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/glow_data.hpp"
#include "pragma/rendering/renderers/rasterization/hdr_data.hpp"
#include <pragma/math/plane.h>
#include <sharedutils/util_weak_handle.hpp>
#include <misc/types.h>
#include <string>
#include <vector>

namespace prosper
{
	class Texture;
	class DescriptorSetGroup;
	class PrimaryCommandBuffer;
};
namespace pragma
{
	class ShaderTextured3D;
	class ShaderPrepassBase;
	class CLightComponent;
	class CParticleSystemComponent;
	class OcclusionMeshInfo;
	class OcclusionCullingHandler;
};
namespace Anvil
{
	class DescriptorSet;
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

		struct DLLCLIENT LightMapInfo
		{
			std::shared_ptr<prosper::Texture> lightMapTexture = nullptr;
		};
		virtual ~RasterizationRenderer() override;

		virtual void EndRendering() override;
		virtual void UpdateRenderSettings(pragma::RenderSettings &renderSettings) override;
		virtual void UpdateCameraData(pragma::CameraData &cameraData) override;
		virtual bool ReloadRenderTarget() override;
		virtual void OnEntityAddedToScene(CBaseEntity &ent) override;
		virtual const std::shared_ptr<prosper::Texture> &GetSceneTexture() const override;
		virtual const std::shared_ptr<prosper::Texture> &GetPresentationTexture() const override;
		virtual bool IsRasterizationRenderer() const override;

		void SetPrepassMode(PrepassMode mode);
		PrepassMode GetPrepassMode() const;

		void SetLightMap(const std::shared_ptr<prosper::Texture> &lightMapTexture);
		const std::shared_ptr<prosper::Texture> &GetLightMap() const;

		void SetShaderOverride(const std::string &srcShader,const std::string &shaderOverride);
		pragma::ShaderTextured3D *GetShaderOverride(pragma::ShaderTextured3D *srcShader);
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
		Anvil::DescriptorSet *GetCSMDescriptorSet() const;
		//Anvil::DescriptorSet *GetLightSourceDescriptorSet() const;

		Float GetHDRExposure() const;
		Float GetMaxHDRExposure() const;
		void SetMaxHDRExposure(Float exposure);
		HDRData &GetHDRInfo();
		GlowData &GetGlowInfo();
		SSAOInfo &GetSSAOInfo();

		Anvil::DescriptorSet *GetDepthDescriptorSet() const;
		void UpdateLightDescriptorSets(const std::vector<pragma::CLightComponent*> &lightSources);
		void SetFogOverride(const std::shared_ptr<prosper::DescriptorSetGroup> &descSetGroup);

		pragma::rendering::Prepass &GetPrepass();
		const pragma::rendering::ForwardPlusInstance &GetForwardPlusInstance() const;
		pragma::rendering::ForwardPlusInstance &GetForwardPlusInstance();
		CulledMeshData *GetRenderInfo(RenderMode mode) const;
		Anvil::SampleCountFlagBits GetSampleCount() const;
		bool IsMultiSampled() const;

		bool BeginRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,prosper::RenderPass *customRenderPass=nullptr);
		bool EndRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);
		bool ResolveRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);
		void PrepareRendering(RenderMode mode,bool bUpdateTranslucentMeshes=false,bool bUpdateGlowMeshes=false);

		OcclusionOctree<CBaseEntity*> &GetOcclusionOctree();
		const OcclusionOctree<CBaseEntity*> &GetOcclusionOctree() const;

		const pragma::OcclusionCullingHandler &GetOcclusionCullingHandler() const;
		pragma::OcclusionCullingHandler &GetOcclusionCullingHandler();
		void SetOcclusionCullingHandler(const std::shared_ptr<pragma::OcclusionCullingHandler> &handler);
		void ReloadOcclusionCullingHandler();

		pragma::ShaderPrepassBase &GetPrepassShader() const;

		// Render
		void Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,float interpolation,FRender renderFlags);
		void GetRenderEntities(std::vector<CBaseEntity*> &entsRender);
		void RenderParticleSystems(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::vector<pragma::CParticleSystemComponent*> &particles,float interpolation,RenderMode renderMode,Bool bloom=false,std::vector<pragma::CParticleSystemComponent*> *bloomParticles=nullptr);

		// Renders all meshes from m_glowInfo.tmpGlowMeshes, and clears the container when done
		void RenderGlowMeshes(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,RenderMode renderMode);

		// If this flag is set, the prepass depth buffer will be blitted into a sampleable buffer
		// before rendering, which can then be used as shader sampler input. This flag will be reset once
		// rendering has finished.
		void SetFrameDepthBufferSamplingRequired();

		virtual bool RenderScene(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::shared_ptr<prosper::RenderTarget> &rt,FRender renderFlags=FRender::All) override;
	private:
		friend BaseRenderer;
		RasterizationRenderer(Scene &scene);
		void InitializeLightDescriptorSets();
		virtual bool Initialize() override;
		virtual void BeginRendering(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd) override;

		void RenderScenePrepass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);
		void RenderScenePostProcessing(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,FRender renderFlags);
		void RenderSceneFog(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);
		void RenderSceneResolveHDR(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Anvil::DescriptorSet &descSetHdrResolve,bool toneMappingOnly=false);

		StateFlags m_stateFlags = StateFlags::PrepassEnabled;

		Anvil::SampleCountFlagBits m_sampleCount = Anvil::SampleCountFlagBits::_1_BIT;
		std::shared_ptr<pragma::OcclusionCullingHandler> m_occlusionCullingHandler = nullptr;
		std::shared_ptr<OcclusionOctree<CBaseEntity*>> m_occlusionOctree = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupFogOverride = nullptr;

		LightMapInfo m_lightMapInfo = {};
		mutable std::unordered_map<RenderMode,std::shared_ptr<CulledMeshData>> m_renderInfo;
		bool m_bFrameDepthBufferSamplingRequired = false;

		// HDR
		HDRData m_hdrInfo;
		GlowData m_glowInfo;

		// Frustum planes (Required for culling)
		std::vector<Plane> m_frustumPlanes = {};
		std::vector<Plane> m_clippedFrustumPlanes = {};
		void UpdateFrustumPlanes();

		// Culled objects
		std::vector<pragma::OcclusionMeshInfo> m_culledMeshes;
		std::vector<pragma::CParticleSystemComponent*> m_culledParticles;

		std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupCSM;

		std::unordered_map<size_t,::util::WeakHandle<prosper::Shader>> m_shaderOverrides;
		mutable ::util::WeakHandle<prosper::Shader> m_whShaderWireframe = {};
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::RasterizationRenderer::StateFlags)

#endif
