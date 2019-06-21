#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/particlesystem/renderers/c_particle_mod_sprite.h"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_shadow.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/particlesystem/renderers/c_particle_renderer_rotational_buffer.hpp"
#include <buffers/prosper_buffer.hpp>

REGISTER_PARTICLE_RENDERER(sprite,CParticleRendererSprite);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

CParticleRendererSprite::CParticleRendererSprite(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleRenderer(pSystem,values)
{
	auto bAlignVelocity = false;
	for(auto &pair : values)
	{
		auto key = pair.first;
		ustring::to_lower(key);
		if(key == "planar_rotation")
			m_bPlanarRotation = util::to_boolean(pair.second);
		else if(key == "align_velocity")
			bAlignVelocity = util::to_boolean(pair.second);
	}
	m_shader = c_engine->GetShader(m_bPlanarRotation ? "particle" : "particle_rotational");
	if(m_bPlanarRotation == true)
		return;
	m_rotationalBuffer = std::make_unique<CParticleRendererRotationalBuffer>(pSystem);
	m_rotationalBuffer->SetRotationAlignVelocity(bAlignVelocity);
}

void CParticleRendererSprite::PostSimulate(double tDelta)
{
	CParticleRenderer::PostSimulate(tDelta);
	if(m_shader.expired() || m_rotationalBuffer == nullptr)
		return;
	m_rotationalBuffer->Update();
}

void CParticleRendererSprite::Render(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,bool bloom)
{
	auto *shader = static_cast<pragma::ShaderParticle2DBase*>(m_shader.get());
	if(shader == nullptr || shader->BeginDraw(drawCmd,GetParticleSystem()) == false) // prosper TODO: Use unlit pipeline if low shader quality?
		return;
	auto &descSetLightSources = *renderer.GetForwardPlusInstance().GetDescriptorSetGraphics();
	auto &descSetShadows = *renderer.GetCSMDescriptorSet();
	shader->BindLights(descSetShadows,descSetLightSources);
	shader->BindRenderSettings(c_game->GetGlobalRenderSettingsDescriptorSet());
	shader->BindSceneCamera(renderer,(m_particleSystem.GetRenderMode() == RenderMode::View) ? true : false);
	if(m_bPlanarRotation == false)
		static_cast<pragma::ShaderParticleRotational&>(*shader).BindWorldRotationBuffer(**m_rotationalBuffer->GetBuffer());
	shader->Draw(
		renderer,m_particleSystem,
		(m_rotationalBuffer != nullptr && m_rotationalBuffer->ShouldRotationAlignVelocity()) ? pragma::CParticleSystemComponent::OrientationType::Velocity : m_particleSystem.GetOrientationType(),
		bloom
	);
	shader->EndDraw();
}

void CParticleRendererSprite::RenderShadow(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId)
{
	/*static auto hShader = c_engine->GetShader("particleshadow");
	if(!hShader.IsValid())
		return;
	auto &shader = static_cast<Shader::ParticleShadow&>(*hShader.get());
	if(shader.BeginDraw() == false)
		return;
	//auto &cam = *c_game->GetRenderCamera();
	shader.Draw(m_particleSystem,light,layerId);
	shader.EndDraw();*/ // prosper TODO
}
