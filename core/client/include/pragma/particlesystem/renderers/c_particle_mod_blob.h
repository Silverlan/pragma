/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PARTICLE_MOD_BLOB_H__
#define __C_PARTICLE_MOD_BLOB_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_blob.hpp"
#include "pragma/debug/c_debugoverlay.h"

#define ENABLE_BLOB_DEPTH_TEST 0

namespace Shader {
	class ParticleBlobShadow;
};
namespace pragma {
	class CLightComponent;
};
class DLLCLIENT CParticleRendererBlob : public CParticleRenderer {
  private:
	static bool s_bShowNeighborLinks;
	static std::shared_ptr<prosper::IDescriptorSetGroup> s_dsParticles;
	static std::size_t s_activeBlobRendererCount;
	static pragma::ShaderParticleBlob *s_shader;
	static Shader::ParticleBlobShadow *s_shadowShader;
  protected:
	static constexpr auto INVALID_BLOB_INDEX = std::numeric_limits<uint16_t>::max();
	struct Link {
		uint32_t targetParticleIdx = INVALID_BLOB_INDEX;
		float distSqr = std::numeric_limits<float>::max();
	};
	struct LinkContainer {
		std::array<Link, pragma::ShaderParticleBlob::MAX_BLOB_NEIGHBORS> links; // First slot is reserved for own particle index
		uint32_t nextLinkId = 1;
	};
	std::vector<LinkContainer> m_particleLinks;
	Vector4 m_specularColor = {};
	float m_reflectionIntensity = 0.f;
	float m_refractionIndexRatio = 1.f;
	pragma::ShaderParticleBlob::DebugMode m_debugMode = pragma::ShaderParticleBlob::DebugMode::None;
	uint64_t m_lastFrame = std::numeric_limits<uint64_t>::max();
	std::vector<std::array<uint16_t, pragma::ShaderParticleBlob::MAX_BLOB_NEIGHBORS>> m_adjacentParticleIds;
	std::shared_ptr<prosper::IBuffer> m_adjacentBlobBuffer = nullptr;
	//Vulkan::RenderTarget m_rtTransparent = nullptr; // prosper TODO
	void SortParticleLinks();
	void UpdateAdjacentParticles(prosper::ICommandBuffer &cmd, prosper::IBuffer &blobIndexBuffer);

	// Debug
	struct DebugInfo {
		std::array<std::shared_ptr<DebugRenderer::BaseObject>, pragma::ShaderParticleBlob::MAX_BLOB_NEIGHBORS - 1> renderObjects;
		bool hide = false;
	};
	std::vector<DebugInfo> m_dbgNeighborLinks;
	void ShowDebugNeighborLinks(bool b);
	void UpdateDebugNeighborLinks();
  public:
	static void SetShowNeighborLinks(bool b);

	CParticleRendererBlob() = default;
	virtual ~CParticleRendererBlob() override;
	virtual void RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::ParticleRenderFlags renderFlags) override;
	virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId = 0) override;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
	virtual void OnParticleSystemStarted() override;
	virtual void OnParticleDestroyed(CParticle &particle) override;
	virtual void OnParticleSystemStopped() override;
	virtual void PreRender(prosper::ICommandBuffer &cmd) override;
	virtual bool RequiresDepthPass() const override { return true; }
	virtual pragma::ShaderParticleBase *GetShader() const override;
};

#endif
