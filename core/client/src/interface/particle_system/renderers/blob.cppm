// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.renderer_blob;

export import :particle_system.modifier;
import :rendering.shaders;

import :debug;

export namespace pragma::pts {
	class DLLCLIENT CParticleRendererBlob : public CParticleRenderer {
	  private:
		static bool s_bShowNeighborLinks;
		static std::shared_ptr<prosper::IDescriptorSetGroup> s_dsParticles;
		static std::size_t s_activeBlobRendererCount;
		static ShaderParticleBlob *s_shader;
		static Shader::ParticleBlobShadow *s_shadowShader;
	  protected:
		static constexpr auto INVALID_BLOB_INDEX = std::numeric_limits<uint16_t>::max();
		struct Link {
			uint32_t targetParticleIdx = INVALID_BLOB_INDEX;
			float distSqr = std::numeric_limits<float>::max();
		};
		struct LinkContainer {
			std::array<Link, ShaderParticleBlob::MAX_BLOB_NEIGHBORS> links; // First slot is reserved for own particle index
			uint32_t nextLinkId = 1;
		};
		std::vector<LinkContainer> m_particleLinks;
		Vector4 m_specularColor = {};
		float m_reflectionIntensity = 0.f;
		float m_refractionIndexRatio = 1.f;
		ShaderParticleBlob::DebugMode m_debugMode = ShaderParticleBlob::DebugMode::None;
		uint64_t m_lastFrame = std::numeric_limits<uint64_t>::max();
		std::vector<std::array<uint16_t, ShaderParticleBlob::MAX_BLOB_NEIGHBORS>> m_adjacentParticleIds;
		std::shared_ptr<prosper::IBuffer> m_adjacentBlobBuffer = nullptr;
		//Vulkan::RenderTarget m_rtTransparent = nullptr; // prosper TODO
		void SortParticleLinks();
		void UpdateAdjacentParticles(prosper::ICommandBuffer &cmd, prosper::IBuffer &blobIndexBuffer);

		// Debug
		struct DebugInfo {
			std::array<std::shared_ptr<debug::DebugRenderer::BaseObject>, ShaderParticleBlob::MAX_BLOB_NEIGHBORS - 1> renderObjects;
			bool hide = false;
		};
		std::vector<DebugInfo> m_dbgNeighborLinks;
		void ShowDebugNeighborLinks(bool b);
		void UpdateDebugNeighborLinks();
	  public:
		static void SetShowNeighborLinks(bool b);

		CParticleRendererBlob() = default;
		virtual ~CParticleRendererBlob() override;
		virtual void RecordRender(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ParticleRenderFlags renderFlags) override;
		virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CLightComponent &light, uint32_t layerId = 0) override;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
		virtual void OnParticleSystemStarted() override;
		virtual void OnParticleDestroyed(CParticle &particle) override;
		virtual void OnParticleSystemStopped() override;
		virtual void PreRender(prosper::ICommandBuffer &cmd) override;
		virtual bool RequiresDepthPass() const override { return true; }
		virtual ShaderParticleBase *GetShader() const override;
	};
}
