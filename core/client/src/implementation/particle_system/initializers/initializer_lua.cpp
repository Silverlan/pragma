// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :particle_system.initializer_lua;

import :entities.components;

void pragma::pts::CParticleModifierLua::Initialize(const luabind::object &o) { m_baseLuaObj = std::shared_ptr<luabind::object>(new luabind::object(o)); }

void pragma::pts::CParticleModifierLua::SetIdentifier(const std::string &identifier) { m_identifier = identifier; }
const std::string &pragma::pts::CParticleModifierLua::GetIdentifier() const { return m_identifier; }

//////////////

void pragma::pts::CParticleOperatorLua::PreSimulate(CParticle &particle, double tDelta) { TParticleModifierLua<CParticleOperator>::PreSimulate(particle, tDelta); }
void pragma::pts::CParticleOperatorLua::Simulate(CParticle &particle, double tDelta, float strength)
{
	TParticleModifierLua<CParticleOperator>::Simulate(particle, tDelta, strength);
	CallLuaMember<void, std::reference_wrapper<CParticle>, float, float>("Simulate", std::ref(particle), tDelta, strength);
}
void pragma::pts::CParticleOperatorLua::PostSimulate(CParticle &particle, double tDelta) { TParticleModifierLua<CParticleOperator>::PostSimulate(particle, tDelta); }
void pragma::pts::CParticleOperatorLua::Simulate(double tDelta) { TParticleModifierLua<CParticleOperator>::Simulate(tDelta); }

//////////////

void pragma::pts::CParticleRendererLua::RecordRender(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ParticleRenderFlags renderFlags)
{
	// No longer supported
	//CallLuaMember<void,std::reference_wrapper<prosper::ICommandBuffer>,luabind::object,luabind::object,uint32_t>(
	//	"Render",std::ref(*drawCmd),scene.GetLuaObject(),renderer.GetLuaObject(),pragma::math::to_integral(renderFlags)
	//);
}
void pragma::pts::CParticleRendererLua::RecordRenderShadow(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CLightComponent &light, uint32_t layerId)
{
	// TODO
}
pragma::ShaderParticleBase *pragma::pts::CParticleRendererLua::GetShader() const { return m_shader; }
void pragma::pts::CParticleRendererLua::SetShader(ShaderParticleBase *shader) { m_shader = shader; }
