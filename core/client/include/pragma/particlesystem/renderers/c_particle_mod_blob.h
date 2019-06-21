#ifndef __C_PARTICLE_MOD_BLOB_H__
#define __C_PARTICLE_MOD_BLOB_H__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_blob.hpp"
#include "pragma/debug/c_debugoverlay.h"

#define ENABLE_BLOB_DEPTH_TEST 0

namespace Shader {class ParticleBlobShadow;};
namespace pragma {class CLightComponent;};
class DLLCLIENT CParticleRendererBlob
	: public CParticleRenderer
{
private:
	static bool s_bShowNeighborLinks;
	//static Vulkan::DescriptorSet s_dsParticles; // prosper TODO
	static std::size_t s_activeBlobRendererCount;
	//static Shader::ParticleBlob *s_shader; // prosper TODO
	static Shader::ParticleBlobShadow *s_shadowShader;
protected:
	static const auto INVALID_BLOB_INDEX = std::numeric_limits<uint16_t>::max();
	struct Link
	{
		uint32_t targetParticleIdx = INVALID_BLOB_INDEX;
		float distSqr = std::numeric_limits<float>::max();
	};
	struct LinkContainer
	{
		//std::array<Link,Shader::ParticleBlob::MAX_BLOB_NEIGHBORS> links; // First slot is reserved for own particle index // prosper TODO
		uint32_t nextLinkId = 1;
	};
	std::vector<LinkContainer> m_particleLinks;
	Vector4 m_specularColor = {};
	float m_reflectionIntensity = 0.f;
	float m_refractionIndexRatio = 1.f;
	//Shader::ParticleBlob::DebugMode m_debugMode = Shader::ParticleBlob::DebugMode::None; // prosper TODO
	uint64_t m_lastFrame = std::numeric_limits<uint64_t>::max();
	//std::vector<std::array<uint16_t,Shader::ParticleBlob::MAX_BLOB_NEIGHBORS>> m_adjacentParticleIds; // prosper TODO
	//Vulkan::SwapBuffer m_adjacentBlobBuffer = nullptr; // prosper TODO
	//Vulkan::RenderTarget m_rtTransparent = nullptr; // prosper TODO
	void SortParticleLinks();
	//void UpdateAdjacentParticles(const Vulkan::Buffer &blobIndexBuffer); // prosper TODO

	// Debug
	struct DebugInfo
	{
		//std::array<std::shared_ptr<DebugRenderer::BaseObject>,Shader::ParticleBlob::MAX_BLOB_NEIGHBORS -1> renderObjects; // prosper TODO
		bool hide = false;
	};
	std::vector<DebugInfo> m_dbgNeighborLinks;
	void ShowDebugNeighborLinks(bool b);
	void UpdateDebugNeighborLinks();
public:
	static void SetShowNeighborLinks(bool b);

	CParticleRendererBlob(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual ~CParticleRendererBlob() override;
	virtual void Render(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,bool bloom) override;
	virtual void RenderShadow(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId=0) override;
	virtual void Initialize() override;
	virtual void Destroy(CParticle &particle) override;
	virtual void Destroy() override;
};

#endif