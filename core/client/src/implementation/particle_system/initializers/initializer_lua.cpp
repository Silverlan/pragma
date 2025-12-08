// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :particle_system.initializer_lua;

import :entities.components;

void pragma::pts::CParticleModifierLua::Initialize(const luabind::object &o) { m_baseLuaObj = std::shared_ptr<luabind::object>(new luabind::object(o)); }

void pragma::pts::CParticleModifierLua::SetIdentifier(const std::string &identifier) { m_identifier = identifier; }
const std::string &pragma::pts::CParticleModifierLua::GetIdentifier() const { return m_identifier; }

//////////////

void pragma::pts::CParticleOperatorLua::PreSimulate(pragma::pts::CParticle &particle, double tDelta) { TParticleModifierLua<pragma::pts::CParticleOperator>::PreSimulate(particle, tDelta); }
void pragma::pts::CParticleOperatorLua::Simulate(pragma::pts::CParticle &particle, double tDelta, float strength)
{
	TParticleModifierLua<pragma::pts::CParticleOperator>::Simulate(particle, tDelta, strength);
	CallLuaMember<void, std::reference_wrapper<pragma::pts::CParticle>, float, float>("Simulate", std::ref(particle), tDelta, strength);
}
void pragma::pts::CParticleOperatorLua::PostSimulate(pragma::pts::CParticle &particle, double tDelta) { TParticleModifierLua<pragma::pts::CParticleOperator>::PostSimulate(particle, tDelta); }
void pragma::pts::CParticleOperatorLua::Simulate(double tDelta) { TParticleModifierLua<pragma::pts::CParticleOperator>::Simulate(tDelta); }

//////////////

void pragma::pts::CParticleRendererLua::RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::pts::ParticleRenderFlags renderFlags)
{
	// No longer supported
	//CallLuaMember<void,std::reference_wrapper<prosper::ICommandBuffer>,luabind::object,luabind::object,uint32_t>(
	//	"Render",std::ref(*drawCmd),scene.GetLuaObject(),renderer.GetLuaObject(),umath::to_integral(renderFlags)
	//);
}
void pragma::pts::CParticleRendererLua::RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId)
{
	// TODO
}
pragma::ShaderParticleBase *pragma::pts::CParticleRendererLua::GetShader() const { return m_shader; }
void pragma::pts::CParticleRendererLua::SetShader(pragma::ShaderParticleBase *shader) { m_shader = shader; }
