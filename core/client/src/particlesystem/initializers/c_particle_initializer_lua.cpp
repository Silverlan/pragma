/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/particlesystem/initializers/c_particle_initializer_lua.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include <prosper_command_buffer.hpp>

void CParticleModifierLua::Initialize(const luabind::object &o) { m_baseLuaObj = std::shared_ptr<luabind::object>(new luabind::object(o)); }

void CParticleModifierLua::SetIdentifier(const std::string &identifier) { m_identifier = identifier; }
const std::string &CParticleModifierLua::GetIdentifier() const { return m_identifier; }

//////////////

void CParticleOperatorLua::PreSimulate(CParticle &particle, double tDelta) { TParticleModifierLua<CParticleOperator>::PreSimulate(particle, tDelta); }
void CParticleOperatorLua::Simulate(CParticle &particle, double tDelta, float strength)
{
	TParticleModifierLua<CParticleOperator>::Simulate(particle, tDelta, strength);
	CallLuaMember<void, std::reference_wrapper<CParticle>, float, float>("Simulate", std::ref(particle), tDelta, strength);
}
void CParticleOperatorLua::PostSimulate(CParticle &particle, double tDelta) { TParticleModifierLua<CParticleOperator>::PostSimulate(particle, tDelta); }
void CParticleOperatorLua::Simulate(double tDelta) { TParticleModifierLua<CParticleOperator>::Simulate(tDelta); }

//////////////

void CParticleRendererLua::RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::ParticleRenderFlags renderFlags)
{
	// No longer supported
	//CallLuaMember<void,std::reference_wrapper<prosper::ICommandBuffer>,luabind::object,luabind::object,uint32_t>(
	//	"Render",std::ref(*drawCmd),scene.GetLuaObject(),renderer.GetLuaObject(),umath::to_integral(renderFlags)
	//);
}
void CParticleRendererLua::RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId)
{
	// TODO
}
pragma::ShaderParticleBase *CParticleRendererLua::GetShader() const { return m_shader; }
void CParticleRendererLua::SetShader(pragma::ShaderParticleBase *shader) { m_shader = shader; }
