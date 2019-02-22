#include "stdafx_client.h"
#include "pragma/particlesystem/c_particlemodifier.h"
#include <algorithm>
#include <mathutil/umath.h>
#include <pragma/math/vector/wvvector3.h>
#include <sharedutils/util.h>
#include <sharedutils/util_string.h>

class DLLCLIENT CParticleInitializerLifetimeRandom
	: public CParticleInitializer
{
private:
	float m_lifeMin;
	float m_lifeMax;
public:
	CParticleInitializerLifetimeRandom(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
		: CParticleInitializer(pSystem,values),
		m_lifeMin(0),m_lifeMax(0)
	{
		for(auto it=values.begin();it!=values.end();it++)
		{
			std::string key = it->first;
			StringToLower(key);
			if(key == "lifetime_min")
				m_lifeMin = util::to_float(it->second);
			else if(key == "lifetime_max")
				m_lifeMax = util::to_float(it->second);
		}
	}
	void Initialize(CParticle &particle)
	{
		particle.SetLife(umath::random(m_lifeMin,m_lifeMax));
	}
};
REGISTER_PARTICLE_INITIALIZER(lifetime_random,CParticleInitializerLifetimeRandom);

class DLLCLIENT CParticleInitializerColorRandom
	: public CParticleInitializer
{
private:
	Color m_colorA;
	Color m_colorB;
	std::unique_ptr<Color> m_colorC;
public:
	CParticleInitializerColorRandom(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
		: CParticleInitializer(pSystem,values),
		m_colorA(255,255,255,255),m_colorB(255,255,255,255)
	{
		for(auto it=values.begin();it!=values.end();it++)
		{
			std::string key = it->first;
			StringToLower(key);
			if(key == "color1")
				m_colorA = Color(it->second);
			else if(key == "color2")
				m_colorB = Color(it->second);
			else if(key == "color3")
				m_colorC = std::make_unique<Color>(it->second);
		}
	}
	void Initialize(CParticle &particle)
	{
		auto col = m_colorA.Lerp(m_colorB,umath::random(0.f,1.f));
		if(m_colorC != nullptr)
			col = col.Lerp(*m_colorC,umath::random(0.f,1.f));
		particle.SetColor(col);
	}
};
REGISTER_PARTICLE_INITIALIZER(color_random,CParticleInitializerColorRandom);

class DLLCLIENT CParticleInitializerAlphaRandom
	: public CParticleInitializer
{
private:
	float m_alphaMin;
	float m_alphaMax;
public:
	CParticleInitializerAlphaRandom(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
		: CParticleInitializer(pSystem,values),
		m_alphaMin(0.f),m_alphaMax(255.f)
	{
		for(auto it=values.begin();it!=values.end();it++)
		{
			std::string key = it->first;
			StringToLower(key);
			if(key == "alpha_min")
				m_alphaMin = util::to_float(it->second);
			else if(key == "alpha_max")
				m_alphaMax = util::to_float(it->second);
		}
	}
	void Initialize(CParticle &particle)
	{
		Color &col = particle.GetColor();
		col.a = CInt16(umath::random(m_alphaMin,m_alphaMax));
	}
};
REGISTER_PARTICLE_INITIALIZER(alpha_random,CParticleInitializerAlphaRandom);

class DLLCLIENT CParticleInitializerRotationRandom
	: public CParticleInitializer
{
private:
	EulerAngles m_rotMin = EulerAngles(-180.f,-180.f,-180.f);
	EulerAngles m_rotMax = EulerAngles(180.f,180.f,180.f);
	Quat m_rot = {};
	float m_planarRotMin = 0.f;
	float m_planarRotMax = 0.f;
	bool m_bUseQuaternionRotation = false;
public:
	CParticleInitializerRotationRandom(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
		: CParticleInitializer(pSystem,values)
	{
		for(auto it=values.begin();it!=values.end();it++)
		{
			std::string key = it->first;
			StringToLower(key);
			if(key == "rotation_quat")
			{
				m_rot = uquat::create(it->second);
				m_bUseQuaternionRotation = true;
			}
			else if(key == "rotation_min")
			{
				m_rotMin = EulerAngles(it->second);
				m_planarRotMin = util::to_float(it->second);
			}
			else if(key == "rotation_max")
			{
				m_rotMax = EulerAngles(it->second);
				m_planarRotMax = util::to_float(it->second);
			}
		}
	}
	void Initialize(CParticle &particle)
	{
		if(m_bUseQuaternionRotation == true)
		{
			particle.SetWorldRotation(m_rot);
			return;
		}
		particle.SetWorldRotation(uquat::create(EulerAngles(umath::random(m_rotMin.p,m_rotMax.p),umath::random(m_rotMin.y,m_rotMax.y),umath::random(m_rotMin.r,m_rotMax.r))));
		particle.SetRotation(umath::random(m_planarRotMin,m_planarRotMax));
	}
};
REGISTER_PARTICLE_INITIALIZER(rotation_random,CParticleInitializerRotationRandom);

///////////////////////

CParticleModifier::CParticleModifier(pragma::CParticleSystemComponent &pSystem)
	: m_particleSystem(pSystem)
{}

CParticleModifier::~CParticleModifier()
{}

pragma::CParticleSystemComponent &CParticleModifier::GetParticleSystem() {return m_particleSystem;}

void CParticleModifier::Initialize(CParticle&) {}
void CParticleModifier::Initialize() {}
void CParticleModifier::Destroy(CParticle&) {}
void CParticleModifier::Destroy() {}
void CParticleModifier::SetName(const std::string &name) {m_name = name;}
const std::string &CParticleModifier::GetName() const {return m_name;}

///////////////////////

CParticleInitializer::CParticleInitializer(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string>&)
	: CParticleModifier(pSystem)
{}

CParticleInitializer::~CParticleInitializer()
{}

///////////////////////

CParticleOperator::CParticleOperator(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string>&)
	: CParticleModifier(pSystem)
{}

CParticleOperator::~CParticleOperator()
{}

void CParticleOperator::Simulate(double)
{}

void CParticleOperator::PreSimulate(CParticle &particle,double tDelta)
{}

void CParticleOperator::Simulate(CParticle&,double)
{}

void CParticleOperator::PostSimulate(CParticle &particle,double tDelta)
{}

CParticleOperatorLifespanDecay::CParticleOperatorLifespanDecay(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleOperator(pSystem,values)
{
#pragma message ("TODO: Actually implement this?")
}

void CParticleOperatorLifespanDecay::Simulate(CParticle&,double)
{}

///////////////////////

CParticleRenderer::CParticleRenderer(pragma::CParticleSystemComponent &pSystem,const std::unordered_map<std::string,std::string> &values)
	: CParticleModifier(pSystem)
{}

CParticleRenderer::~CParticleRenderer()
{}

void CParticleRenderer::PostSimulate(double tDelta) {}

std::pair<Vector3,Vector3> CParticleRenderer::GetRenderBounds() const {return {uvec::ORIGIN,uvec::ORIGIN};}

///////////////////////

DLLCLIENT ParticleModifierMap *g_ParticleModifierFactories = NULL;
DLLCLIENT void LinkParticleInitializerToFactory(std::string name,std::unique_ptr<CParticleInitializer>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&))
{
	if(g_ParticleModifierFactories == NULL)
	{
		static ParticleModifierMap map;
		g_ParticleModifierFactories = &map;
	}
	g_ParticleModifierFactories->AddInitializer(name,fc);
}
DLLCLIENT void LinkParticleOperatorToFactory(std::string name,std::unique_ptr<CParticleOperator>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&))
{
	if(g_ParticleModifierFactories == NULL)
	{
		static ParticleModifierMap map;
		g_ParticleModifierFactories = &map;
	}
	g_ParticleModifierFactories->AddOperator(name,fc);
}
DLLCLIENT void LinkParticleRendererToFactory(std::string name,std::unique_ptr<CParticleRenderer>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&))
{
	if(g_ParticleModifierFactories == NULL)
	{
		static ParticleModifierMap map;
		g_ParticleModifierFactories = &map;
	}
	g_ParticleModifierFactories->AddRenderer(name,fc);
}
DLLCLIENT ParticleModifierMap *GetParticleModifierMap() {return g_ParticleModifierFactories;}

void ParticleModifierMap::AddInitializer(std::string name,std::unique_ptr<CParticleInitializer>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&))
{
	StringToLower(name);
	m_initializers.insert(std::unordered_map<std::string,std::unique_ptr<CParticleInitializer>(*)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&)>::value_type(name,fc));
}
void ParticleModifierMap::AddOperator(std::string name,std::unique_ptr<CParticleOperator>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&))
{
	StringToLower(name);
	m_operators.insert(std::unordered_map<std::string,std::unique_ptr<CParticleOperator>(*)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&)>::value_type(name,fc));
}
void ParticleModifierMap::AddRenderer(std::string name,std::unique_ptr<CParticleRenderer>(*fc)(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&))
{
	StringToLower(name);
	m_renderers.insert(decltype(m_renderers)::value_type(name,fc));
}

std::unique_ptr<CParticleInitializer >(*ParticleModifierMap::FindInitializer(std::string classname))(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&)
{
	auto it = m_initializers.find(classname);
	if(it == m_initializers.end())
		return NULL;
	return it->second;
}
std::unique_ptr<CParticleOperator >(*ParticleModifierMap::FindOperator(std::string classname))(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&)
{
	auto it = m_operators.find(classname);
	if(it == m_operators.end())
		return NULL;
	return it->second;
}
std::unique_ptr<CParticleRenderer >(*ParticleModifierMap::FindRenderer(std::string classname))(pragma::CParticleSystemComponent&,const std::unordered_map<std::string,std::string>&)
{
	auto it = m_renderers.find(classname);
	if(it == m_renderers.end())
		return NULL;
	return it->second;
}