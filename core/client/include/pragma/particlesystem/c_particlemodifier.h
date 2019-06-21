#ifndef __C_PARTICLEMODIFIER_H__
#define __C_PARTICLEMODIFIER_H__

#include "pragma/particlesystem/c_particle.h"
#include <unordered_map>
#include <cmaterial.h>

namespace pragma {class CParticleSystemComponent;};
#define REGISTER_PARTICLE_MODIFIER(localname,classname,basetype) \
	static std::unique_ptr<basetype> CreateParticle##classname##Modifier(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values) \
	{ \
		auto r = std::make_unique<classname>(pSystem,values); \
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

#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT CParticleModifier
	: public CParticleSystemBaseKeyValues
{
public:
	const std::string &GetName() const;
	void SetName(const std::string &name);
	// Called when a new particle has been created
	virtual void Initialize(CParticle &particle);
	// Called when the particle system has been started
	virtual void Initialize();
	// Called when a particle has been destroyed
	virtual void Destroy(CParticle &particle);
	// Called when the particle system has been stopped
	virtual void Destroy();
	pragma::CParticleSystemComponent &GetParticleSystem();
protected:
	virtual ~CParticleModifier();
	pragma::CParticleSystemComponent &m_particleSystem;
	CParticleModifier(pragma::CParticleSystemComponent &pSystem);
private:
	std::string m_name;
};

///////////////////////

class DLLCLIENT CParticleInitializer
	: public CParticleModifier
{
protected:
public:
	CParticleInitializer(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual ~CParticleInitializer() override;
};

///////////////////////

class DLLCLIENT CParticleOperator
	: public CParticleModifier
{
protected:
public:
	CParticleOperator(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual ~CParticleOperator() override;
	virtual void PreSimulate(CParticle &particle,double tDelta);
	virtual void Simulate(CParticle &particle,double tDelta);
	virtual void PostSimulate(CParticle &particle,double tDelta);
	virtual void Simulate(double tDelta);
};

class DLLCLIENT CParticleOperatorLifespanDecay
	: public CParticleOperator
{
public:
	CParticleOperatorLifespanDecay(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	void Simulate(CParticle &particle,double tDelta);
};

///////////////////////

class Scene;
namespace pragma
{
	class CLightComponent;
	namespace rendering {class RasterizationRenderer;};
};
class DLLCLIENT CParticleRenderer
	: public CParticleModifier
{
public:
	CParticleRenderer(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values);
	virtual ~CParticleRenderer() override;
	virtual void Render(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,bool bloom)=0;
	virtual void RenderShadow(const std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent &light,uint32_t layerId=0)=0;
	virtual void PostSimulate(double tDelta);
	virtual std::pair<Vector3,Vector3> GetRenderBounds() const;
};
#pragma warning(pop)

///////////////////////

DLLCLIENT void LinkParticleInitializerToFactory(std::string name,std::unique_ptr<CParticleInitializer>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&));
DLLCLIENT void LinkParticleOperatorToFactory(std::string name,std::unique_ptr<CParticleOperator>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&));
DLLCLIENT void LinkParticleRendererToFactory(std::string name,std::unique_ptr<CParticleRenderer>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&));

class DLLCLIENT __reg_particle_modifier
{
public:
	__reg_particle_modifier(std::string name,std::unique_ptr<CParticleInitializer>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&))
	{
		LinkParticleInitializerToFactory(name,fc);
		delete this;
	}
	__reg_particle_modifier(std::string name,std::unique_ptr<CParticleOperator>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&))
	{
		LinkParticleOperatorToFactory(name,fc);
		delete this;
	}
	__reg_particle_modifier(std::string name,std::unique_ptr<CParticleRenderer>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&))
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
	std::unordered_map<std::string,std::unique_ptr<CParticleInitializer>(*)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&)> m_initializers;
	std::unordered_map<std::string,std::unique_ptr<CParticleOperator>(*)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&)> m_operators;
	std::unordered_map<std::string,std::unique_ptr<CParticleRenderer>(*)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&)> m_renderers;
public:
	void AddInitializer(std::string name,std::unique_ptr<CParticleInitializer>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&));
	void AddOperator(std::string name,std::unique_ptr<CParticleOperator>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&));
	void AddRenderer(std::string name,std::unique_ptr<CParticleRenderer>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&));
	std::unique_ptr<CParticleInitializer>(*FindInitializer(std::string classname))(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&);
	std::unique_ptr<CParticleOperator>(*FindOperator(std::string classname))(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&);
	std::unique_ptr<CParticleRenderer>(*FindRenderer(std::string classname))(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&);
};
#pragma warning(pop)

DLLCLIENT ParticleModifierMap *GetParticleModifierMap();

#endif