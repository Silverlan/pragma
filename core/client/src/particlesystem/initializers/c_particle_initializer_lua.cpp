#include "stdafx_client.h"
#include "pragma/particlesystem/initializers/c_particle_initializer_lua.hpp"

#pragma optimize("",off)
void CParticleModifierLua::Initialize(const luabind::object &o)
{
	m_baseLuaObj = std::shared_ptr<luabind::object>(new luabind::object(o));
	CallLuaMember("Initialize");
}

void CParticleModifierLua::SetIdentifier(const std::string &identifier) {m_identifier = identifier;}
const std::string &CParticleModifierLua::GetIdentifier() const {return m_identifier;}

//////////////

void CParticleOperatorLua::PreSimulate(CParticle &particle,double tDelta)
{
	TParticleModifierLua<CParticleOperator>::PreSimulate(particle,tDelta);
}
void CParticleOperatorLua::Simulate(CParticle &particle,double tDelta)
{
	TParticleModifierLua<CParticleOperator>::Simulate(particle,tDelta);
	CallLuaMember<void,std::reference_wrapper<CParticle>,float>("Simulate",std::ref(particle),tDelta);
}
void CParticleOperatorLua::PostSimulate(CParticle &particle,double tDelta)
{
	TParticleModifierLua<CParticleOperator>::PostSimulate(particle,tDelta);
}
void CParticleOperatorLua::Simulate(double tDelta)
{
	TParticleModifierLua<CParticleOperator>::Simulate(tDelta);
}

//////////////

void CParticleRendererLua::Render(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,bool bloom)
{

}
void CParticleRendererLua::RenderShadow(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId)
{

}
#pragma optimize("",on)
