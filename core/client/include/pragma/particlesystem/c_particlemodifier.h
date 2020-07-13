/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_PARTICLEMODIFIER_H__
#define __C_PARTICLEMODIFIER_H__

#include "pragma/particlesystem/c_particle.h"
#include <unordered_map>
#include <cmaterial.h>

namespace pragma {class CParticleSystemComponent;};
#define REGISTER_PARTICLE_MODIFIER(localname,classname,basetype) \
	static std::unique_ptr<basetype,void(*)(basetype*)> CreateParticle##classname##Modifier(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) \
	{ \
		auto r = std::unique_ptr<basetype,void(*)(basetype*)>(new classname{},[](basetype *p) {delete p;}); \
		r->Initialize(pSystem,values); \
		r->SetName(#localname); \
		return r; \
	} \
	__reg_particle_modifier *__reg_particle_modifier_##classname = new __reg_particle_modifier(#localname,&CreateParticle##classname##Modifier);

#define REGISTER_PARTICLE_INITIALIZER(localname,classname) \
	REGISTER_PARTICLE_MODIFIER(localname,classname,CParticleInitializer);
#define REGISTER_PARTICLE_OPERATOR(localname,classname) \
	REGISTER_PARTICLE_MODIFIER(localname,classname,CParticleOperator);
#define REGISTER_PARTICLE_RENDERER(localname,classname) \
	REGISTER_PARTICLE_MODIFIER(localname,classname,CParticleRenderer);

namespace pragma {enum class ParticleRenderFlags : uint32_t;};

#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT CParticleModifier
	: public CParticleSystemBaseKeyValues
{
public:
	const std::string &GetName() const;
	void SetName(const std::string &name);
	const std::string &GetType() const;
	void SetType(const std::string &type);
	// Called when a new particle has been created
	virtual void OnParticleCreated(CParticle &particle);
	// Called when the particle system has been started
	virtual void OnParticleSystemStarted();
	// Called when a particle has been destroyed
	virtual void OnParticleDestroyed(CParticle &particle);
	// Called when the particle system has been stopped
	virtual void OnParticleSystemStopped();
	pragma::CParticleSystemComponent &GetParticleSystem() const;
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);

	int32_t GetPriority() const;
	void SetPriority(int32_t priority);
protected:
	virtual ~CParticleModifier()=default;
	mutable pragma::CParticleSystemComponent *m_particleSystem = nullptr;
	CParticleModifier()=default;
private:
	std::string m_name;
	std::string m_type;
	int32_t m_priority = 0;
};

///////////////////////

class DLLCLIENT CParticleInitializer
	: public CParticleModifier
{
public:
	CParticleInitializer()=default;
};

///////////////////////

class DLLCLIENT CParticleOperator
	: public CParticleModifier
{
public:
	CParticleOperator()=default;
	virtual void PreSimulate(CParticle &particle,double tDelta);
	void Simulate(CParticle &particle,double tDelta);
	virtual void PostSimulate(CParticle &particle,double tDelta);
	virtual void Simulate(double tDelta);
	virtual void Simulate(CParticle &particle,double tDelta,float strength);
	virtual void Initialize(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) override;
	float CalcStrength(float curTime) const;
private:
	float m_opStartFadein = 0.f;
	float m_opEndFadein = 0.f;
	float m_opStartFadeout = 0.f;
	float m_opEndFadeout = 0.f;
	float m_opFadeOscillate = 0.f;
};

class DLLCLIENT CParticleOperatorLifespanDecay
	: public CParticleOperator
{
public:
	CParticleOperatorLifespanDecay()=default;
	virtual void Simulate(CParticle &particle,double tDelta,float strength) override;
};

///////////////////////

class Scene;
namespace pragma
{
	class CLightComponent;
	namespace rendering {class RasterizationRenderer;};
	class ShaderParticleBase;
};
class DLLCLIENT CParticleRenderer
	: public CParticleModifier
{
public:
	CParticleRenderer()=default;
	virtual void Render(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::ParticleRenderFlags renderFlags)=0;
	virtual void RenderShadow(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId=0)=0;
	virtual void PostSimulate(double tDelta);
	virtual std::pair<Vector3,Vector3> GetRenderBounds() const;
	virtual pragma::ShaderParticleBase *GetShader() const=0;
};
#pragma warning(pop)

///////////////////////

template<class T>
	using TParticleModifierFactory = std::function<std::unique_ptr<T,void(*)(T*)>(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&)>;
DLLCLIENT void LinkParticleInitializerToFactory(std::string name,const TParticleModifierFactory<CParticleInitializer> &fc);
DLLCLIENT void LinkParticleOperatorToFactory(std::string name,const TParticleModifierFactory<CParticleOperator> &fc);
DLLCLIENT void LinkParticleRendererToFactory(std::string name,const TParticleModifierFactory<CParticleRenderer> &fc);

class DLLCLIENT __reg_particle_modifier
{
public:
	__reg_particle_modifier(std::string name,const TParticleModifierFactory<CParticleInitializer> &fc)
	{
		LinkParticleInitializerToFactory(name,fc);
		delete this;
	}
	__reg_particle_modifier(std::string name,const TParticleModifierFactory<CParticleOperator> &fc)
	{
		LinkParticleOperatorToFactory(name,fc);
		delete this;
	}
	__reg_particle_modifier(std::string name,const TParticleModifierFactory<CParticleRenderer> &fc)
	{
		LinkParticleRendererToFactory(name,fc);
		delete this;
	}
};

#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT ParticleModifierMap
{
private:
	std::unordered_map<std::string,TParticleModifierFactory<CParticleInitializer>> m_initializers;
	std::unordered_map<std::string,TParticleModifierFactory<CParticleOperator>> m_operators;
	std::unordered_map<std::string,TParticleModifierFactory<CParticleRenderer>> m_renderers;
public:
	void AddInitializer(std::string name,const TParticleModifierFactory<CParticleInitializer> &fc);
	void AddOperator(std::string name,const TParticleModifierFactory<CParticleOperator> &fc);
	void AddRenderer(std::string name,const TParticleModifierFactory<CParticleRenderer> &fc);
	TParticleModifierFactory<CParticleInitializer> FindInitializer(std::string classname);
	TParticleModifierFactory<CParticleOperator> FindOperator(std::string classname);
	TParticleModifierFactory<CParticleRenderer> FindRenderer(std::string classname);

	const std::unordered_map<std::string,TParticleModifierFactory<CParticleInitializer>> &GetInitializers() const;
	const std::unordered_map<std::string,TParticleModifierFactory<CParticleOperator>> &GetOperators() const;
	const std::unordered_map<std::string,TParticleModifierFactory<CParticleRenderer>> &GetRenderers() const;
};
#pragma warning(pop)

DLLCLIENT ParticleModifierMap *GetParticleModifierMap();

#endif