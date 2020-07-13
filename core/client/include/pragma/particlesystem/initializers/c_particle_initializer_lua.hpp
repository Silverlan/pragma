/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_PARTICLE_INITIALIZER_LUA_HPP__
#define __C_PARTICLE_INITIALIZER_LUA_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include <pragma/lua/luaobjectbase.h>

class DLLCLIENT CParticleModifierLua
	: public LuaObjectBase
{
public:
	void Initialize(const luabind::object &o);

	void SetIdentifier(const std::string &identifier);
	const std::string &GetIdentifier() const;
private:
	std::string m_identifier = "";
};

template<class TModifier>
	class TParticleModifierLua
		: public TModifier,public CParticleModifierLua
{
public:
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	virtual void OnParticleCreated(CParticle &particle) override;
	virtual void OnParticleDestroyed(CParticle &particle) override;
	virtual void OnParticleSystemStarted() override;
	virtual void OnParticleSystemStopped() override;

	void Lua_Initialize() {}
	static void Lua_default_Initialize(lua_State *l,TParticleModifierLua<TModifier> &mod) {mod.Lua_Initialize();}

	void Lua_OnParticleSystemStarted() {}
	static void Lua_default_OnParticleSystemStarted(lua_State *l,TParticleModifierLua<TModifier> &mod) {mod.Lua_OnParticleSystemStarted();}

	void Lua_OnParticleSystemStopped() {}
	static void Lua_default_OnParticleSystemStopped(lua_State *l,TParticleModifierLua<TModifier> &mod) {mod.Lua_OnParticleSystemStopped();}

	void Lua_OnParticleCreated(CParticle &pt) {}
	static void Lua_default_OnParticleCreated(lua_State *l,TParticleModifierLua<TModifier> &mod,CParticle &pt) {mod.Lua_OnParticleCreated(pt);}

	void Lua_OnParticleDestroyed(CParticle &pt) {}
	static void Lua_default_OnParticleDestroyed(lua_State *l,TParticleModifierLua<TModifier> &mod,CParticle &pt) {mod.Lua_OnParticleDestroyed(pt);}
};

template<class TModifier>
	void TParticleModifierLua<TModifier>::Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
{
	TModifier::Initialize(pSystem,values);
	RecordKeyValues(values);
	CallLuaMember("Initialize");
}

template<class TModifier>
	void TParticleModifierLua<TModifier>::OnParticleCreated(CParticle &particle)
{
	TModifier::OnParticleCreated(particle);
	CallLuaMember<void,std::reference_wrapper<CParticle>>("OnParticleCreated",std::ref(particle));
}
template<class TModifier>
	void TParticleModifierLua<TModifier>::OnParticleDestroyed(CParticle &particle)
{
	TModifier::OnParticleDestroyed(particle);
	CallLuaMember<void,std::reference_wrapper<CParticle>>("OnParticleDestroyed",std::ref(particle));
}
template<class TModifier>
	void TParticleModifierLua<TModifier>::OnParticleSystemStarted()
{
	TModifier::OnParticleSystemStarted();
	CallLuaMember<void>("OnParticleSystemStarted");
}
template<class TModifier>
	void TParticleModifierLua<TModifier>::OnParticleSystemStopped()
{
	TModifier::OnParticleSystemStopped();
	CallLuaMember<void>("OnParticleSystemStopped");
}

class DLLCLIENT CParticleInitializerLua
	: public TParticleModifierLua<CParticleInitializer>
{
public:
	CParticleInitializerLua()=default;
};

class DLLCLIENT CParticleOperatorLua
	: public TParticleModifierLua<CParticleOperator>
{
public:
	CParticleOperatorLua()=default;
	virtual void PreSimulate(CParticle &particle,double tDelta) override;
	virtual void Simulate(CParticle &particle,double tDelta,float strength) override;
	virtual void PostSimulate(CParticle &particle,double tDelta) override;
	virtual void Simulate(double tDelta) override;

	void Lua_Simulate(CParticle &pt,float dt,float strength) {}
	static void Lua_default_Simulate(lua_State *l,CParticleOperatorLua &mod,CParticle &pt,float dt,float strength) {mod.Lua_Simulate(pt,dt,strength);}
};

class DLLCLIENT CParticleRendererLua
	: public TParticleModifierLua<CParticleRenderer>
{
public:
	CParticleRendererLua()=default;
	virtual void Render(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::ParticleRenderFlags renderFlags) override;
	virtual void RenderShadow(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId=0) override;
	virtual pragma::ShaderParticleBase *GetShader() const override;
	void SetShader(pragma::ShaderParticleBase *shader);

	void Lua_Render(prosper::ICommandBuffer &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,bool bloom) {}
	static void Lua_default_Render(lua_State *l,CParticleRendererLua &mod,prosper::ICommandBuffer &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,bool bloom) {mod.Lua_Render(drawCmd,renderer,bloom);}
private:
	pragma::ShaderParticleBase *m_shader = nullptr;
};

#endif
