#include "stdafx_shared.h"
#include <pragma/engine.h>
#include "pragma/math/surfacematerial.h"
#include <fsys/filesystem.h>
#include "datasystem.h"
#include "pragma/ai/navsystem.h"
#include <algorithm>
#include <pragma/physics/physapi.h>

extern DLLENGINE Engine *engine;

SurfaceMaterialManager::SurfaceMaterialManager()
{
	m_materials.push_back(SurfaceMaterial("generic",0,0.8f,0.f));
	auto &surface = m_materials.back();
	surface.SetSoftImpactSound("fx.phys_impact_generic_soft");
	surface.SetHardImpactSound("fx.phys_impact_generic_hard");
	surface.SetBulletImpactSound("fx.phys_impact_generic_bullet");
	surface.SetImpactParticleEffect("impact_dust");
}

void SurfaceMaterialManager::Load(const std::string &path)
{
	auto data = ds::System::LoadData(path.c_str(),{
		{"NAV_FLAG_NONE",std::to_string(umath::to_integral(pragma::nav::PolyFlags::None))},
		{"NAV_FLAG_WALK",std::to_string(umath::to_integral(pragma::nav::PolyFlags::Walk))},
		{"NAV_FLAG_SWIM",std::to_string(umath::to_integral(pragma::nav::PolyFlags::Swim))},
		{"NAV_FLAG_DOOR",std::to_string(umath::to_integral(pragma::nav::PolyFlags::Door))},
		{"NAV_FLAG_JUMP",std::to_string(umath::to_integral(pragma::nav::PolyFlags::Jump))},
		{"NAV_FLAG_DISABLED",std::to_string(umath::to_integral(pragma::nav::PolyFlags::Disabled))},
		{"NAV_FLAG_ALL",std::to_string(umath::to_integral(pragma::nav::PolyFlags::All))}
	});
	if(data == nullptr)
		return;
	auto *values = data->GetData();
	for(auto it=values->begin();it!=values->end();it++)
	{
		auto &val = it->second;
		if(val->IsBlock())
		{
			auto &identifier = it->first;
			if(!identifier.empty())
			{
				auto &physMat = Create(identifier);
				auto *mat = static_cast<ds::Block*>(val.get());

				std::string strVal;
				Float val = 0.f;
				Int32 ival = 0;
				if(mat->GetInt("navigation_flags",&ival))
					physMat.SetNavigationFlags(static_cast<pragma::nav::PolyFlags>(ival));
				if(mat->GetString("footsteps",&strVal))
					physMat.SetFootstepType(strVal);
				if(mat->GetString("impact_bullet",&strVal))
					physMat.SetBulletImpactSound(strVal);
				if(mat->GetString("impact_effect",&strVal))
					physMat.SetImpactParticleEffect(strVal);
				if(mat->GetString("impact_soft",&strVal))
					physMat.SetSoftImpactSound(strVal);
				if(mat->GetString("impact_hard",&strVal))
					physMat.SetHardImpactSound(strVal);
				if(mat->GetFloat("density",&val))
					physMat.SetDensity(val);
				if(mat->GetFloat("linear_drag_coefficient",&val))
					physMat.SetLinearDragCoefficient(val);
				if(mat->GetFloat("torque_drag_coefficient",&val))
					physMat.SetTorqueDragCoefficient(val);
				if(mat->GetFloat("wave_stiffness",&val))
					physMat.SetWaveStiffness(val);
				if(mat->GetFloat("wave_propagation",&val))
					physMat.SetWavePropagation(val);
#ifdef PHYS_ENGINE_BULLET
				if(mat->GetFloat("friction",&val))
					physMat.SetFriction(val);
				if(mat->GetFloat("restitution",&val))
					physMat.SetRestitution(val);
#elif PHYS_ENGINE_PHYSX
				float val;
				if(mat->GetFloat("dynamic_friction",&val))
					physMat->SetDynamicFriction(val);
				if(mat->GetFloat("static_friction",&val))
					physMat->SetStaticFriction(val);
				if(mat->GetFloat("restitution",&val))
					physMat->SetRestitution(val);
				bool bVal;
				if(mat->GetBool("disable_friction",&bVal))
					physMat->SetFrictionEnabled(!bVal);
				if(mat->GetBool("disable_strong_friction",&bVal))
					physMat->SetStrongFrictionEnabled(!bVal);
#endif
				auto audio = mat->GetBlock("audio",0u);
				if(audio != nullptr)
				{
					if(audio->GetFloat("low_frequency_absorption",&val))
						physMat.SetAudioLowFrequencyAbsorption(val);
					if(audio->GetFloat("mid_frequency_absorption",&val))
						physMat.SetAudioMidFrequencyAbsorption(val);
					if(audio->GetFloat("high_frequency_absorption",&val))
						physMat.SetAudioHighFrequencyAbsorption(val);
					if(audio->GetFloat("scattering",&val))
						physMat.SetAudioScattering(val);
					if(audio->GetFloat("low_frequency_transmission",&val))
						physMat.SetAudioLowFrequencyTransmission(val);
					if(audio->GetFloat("mid_frequency_transmission",&val))
						physMat.SetAudioMidFrequencyTransmission(val);
					if(audio->GetFloat("high_frequency_transmission",&val))
						physMat.SetAudioHighFrequencyTransmission(val);
				}
			}
		}
	}
}
SurfaceMaterial &SurfaceMaterialManager::Create(const std::string &identifier,Float friction,Float restitution)
{
	auto *mat = GetMaterial(identifier);
	if(mat == nullptr)
	{
		const auto numReserve = 50;
		m_materials.reserve((m_materials.size() /numReserve) *numReserve +numReserve);
		m_materials.push_back(SurfaceMaterial(identifier,m_materials.size(),friction,restitution));
		mat = &m_materials.back();
	}
	return *mat;
}
SurfaceMaterial *SurfaceMaterialManager::GetMaterial(const std::string &id)
{
	auto it = std::find_if(m_materials.begin(),m_materials.end(),[&id](const SurfaceMaterial &mat) {
		return (mat.GetIdentifier() == id) ? true : false;
	});
	if(it == m_materials.end())
		return nullptr;
	return &(*it);
}
std::vector<SurfaceMaterial> &SurfaceMaterialManager::GetMaterials() {return m_materials;}

////////////////////////////////////

SurfaceMaterial::SurfaceMaterial(const SurfaceMaterial &other)
	: m_navigationFlags(pragma::nav::PolyFlags::Walk)
{
	*this = other;
}

#ifdef PHYS_ENGINE_BULLET
SurfaceMaterial::SurfaceMaterial(const std::string &identifier,UInt idx,Float friction,Float restitution)
	: btMaterial(friction,restitution),m_index(idx),m_identifier(identifier),
	m_footstepType("fx.fst_concrete")
{}
#elif PHYS_ENGINE_PHYSX
SurfaceMaterial::SurfaceMaterial()
{
#ifdef PHYS_ENGINE_PHYSX
	physx::PxPhysics *physics = engine->GetPhysics();
	m_material = physics->createMaterial(0.5f,0.5f,0.1f);
#endif
	Reset();
}

SurfaceMaterial::SurfaceMaterial(float,float,float,std::string footstepType)
{
#ifdef PHYS_ENGINE_PHYSX
	physx::PxPhysics *physics = engine->GetPhysics();
	m_material = physics->createMaterial(staticFriction,dynamicFriction,restitution);
#endif
	Reset();
}

SurfaceMaterial::~SurfaceMaterial()
{}
#endif

SurfaceMaterial &SurfaceMaterial::operator=(const SurfaceMaterial &other)
{
	btMaterial::operator=(other);
	m_index = other.m_index;
	m_identifier = other.m_identifier;
	m_footstepType = other.m_footstepType;
	m_softImpactSound = other.m_softImpactSound;
	m_hardImpactSound = other.m_hardImpactSound;
	m_bulletImpactSound = other.m_bulletImpactSound;
	m_impactParticle = other.m_impactParticle;
	m_navigationFlags = other.m_navigationFlags;
	if(other.m_liquidInfo != nullptr)
	{
		m_liquidInfo = std::make_unique<PhysLiquid>();
		*m_liquidInfo = *other.m_liquidInfo;
	}
	m_audioInfo = other.m_audioInfo;
	return *this;
}

void SurfaceMaterial::SetSoftImpactSound(const std::string &snd) {m_softImpactSound = snd;}
const std::string &SurfaceMaterial::GetSoftImpactSound() const {return m_softImpactSound;}
void SurfaceMaterial::SetHardImpactSound(const std::string &snd) {m_hardImpactSound = snd;}
const std::string &SurfaceMaterial::GetHardImpactSound() const {return m_hardImpactSound;}
void SurfaceMaterial::SetBulletImpactSound(const std::string &snd) {m_bulletImpactSound = snd;}
const std::string &SurfaceMaterial::GetBulletImpactSound() const {return m_bulletImpactSound;}
void SurfaceMaterial::SetImpactParticleEffect(const std::string &particle) {m_impactParticle = particle;}
const std::string &SurfaceMaterial::GetImpactParticleEffect() const {return m_impactParticle;}

PhysLiquid &SurfaceMaterial::InitializeLiquid()
{
	if(m_liquidInfo == nullptr)
		m_liquidInfo = std::make_unique<PhysLiquid>();
	return *m_liquidInfo;
}
void SurfaceMaterial::SetDensity(float density) {InitializeLiquid().density = density;}
float SurfaceMaterial::GetDensity() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_DENSITY;
	return m_liquidInfo->density;
}
void SurfaceMaterial::SetLinearDragCoefficient(float coefficient) {InitializeLiquid().linearDragCoefficient = coefficient;}
float SurfaceMaterial::GetLinearDragCoefficient() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_LINEAR_DRAG_COEFFICIENT;
	return m_liquidInfo->linearDragCoefficient;
}
void SurfaceMaterial::SetTorqueDragCoefficient(float coefficient) {InitializeLiquid().torqueDragCoefficient = coefficient;}
float SurfaceMaterial::GetTorqueDragCoefficient() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_TORQUE_DRAG_COEFFICIENT;
	return m_liquidInfo->torqueDragCoefficient;
}
void SurfaceMaterial::SetWaveStiffness(float stiffness) {InitializeLiquid().stiffness = stiffness;}
float SurfaceMaterial::GetWaveStiffness() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_STIFFNESS;
	return m_liquidInfo->stiffness;
}
void SurfaceMaterial::SetWavePropagation(float propagation) {InitializeLiquid().propagation = propagation;}
float SurfaceMaterial::GetWavePropagation() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_PROPAGATION;
	return m_liquidInfo->propagation;
}

const SurfaceMaterial::AudioInfo &SurfaceMaterial::GetAudioInfo() const {return m_audioInfo;}
void SurfaceMaterial::SetAudioInfo(const AudioInfo &info) {m_audioInfo = info;}
void SurfaceMaterial::SetAudioLowFrequencyAbsorption(float absp) {m_audioInfo.lowFreqAbsorption = absp;}
float SurfaceMaterial::GetAudioLowFrequencyAbsorption() const {return m_audioInfo.lowFreqAbsorption;}
void SurfaceMaterial::SetAudioMidFrequencyAbsorption(float absp) {m_audioInfo.midFreqAbsorption = absp;}
float SurfaceMaterial::GetAudioMidFrequencyAbsorption() const {return m_audioInfo.midFreqAbsorption;}
void SurfaceMaterial::SetAudioHighFrequencyAbsorption(float absp) {m_audioInfo.highFreqAbsorption = absp;}
float SurfaceMaterial::GetAudioHighFrequencyAbsorption() const {return m_audioInfo.highFreqAbsorption;}
void SurfaceMaterial::SetAudioScattering(float scattering) {m_audioInfo.scattering = scattering;}
float SurfaceMaterial::GetAudioScattering() const {return m_audioInfo.scattering;}
void SurfaceMaterial::SetAudioLowFrequencyTransmission(float transmission) {m_audioInfo.lowFreqTransmission = transmission;}
float SurfaceMaterial::GetAudioLowFrequencyTransmission() const {return m_audioInfo.lowFreqTransmission;}
void SurfaceMaterial::SetAudioMidFrequencyTransmission(float transmission) {m_audioInfo.midFreqTransmission = transmission;}
float SurfaceMaterial::GetAudioMidFrequencyTransmission() const {return m_audioInfo.midFreqTransmission;}
void SurfaceMaterial::SetAudioHighFrequencyTransmission(float transmission) {m_audioInfo.highFreqTransmission = transmission;}
float SurfaceMaterial::GetAudioHighFrequencyTransmission() const {return m_audioInfo.highFreqTransmission;}

void SurfaceMaterial::Reset()
{
	m_footstepType = "generic";

	m_audioInfo = {};
	m_navigationFlags = pragma::nav::PolyFlags::None;
#ifdef PHYS_ENGINE_BULLET
	m_friction = 0.5f;
	m_restitution = 0.5f;
#elif PHYS_ENGINE_PHYSX
	m_material->setDynamicFriction(0.5f);
	m_material->setStaticFriction(0.5f);
	m_material->setRestitution(0.1f);
	m_material->setFlag(physx::PxMaterialFlag::Enum::eDISABLE_FRICTION,false);
	m_material->setFlag(physx::PxMaterialFlag::Enum::eDISABLE_STRONG_FRICTION,false);
#endif
}
#ifdef PHYS_ENGINE_BULLET
const std::string &SurfaceMaterial::GetIdentifier() const {return m_identifier;}
Float SurfaceMaterial::GetFriction() const {return CFloat(m_friction);}
void SurfaceMaterial::SetFriction(Float friction) {m_friction = friction;}
Float SurfaceMaterial::GetRestitution() const {return CFloat(m_restitution);}
void SurfaceMaterial::SetRestitution(Float restitution) {m_restitution = restitution;}
UInt SurfaceMaterial::GetIndex() const {return m_index;}
#elif PHYS_ENGINE_PHYSX
void SurfaceMaterial::SetDynamicFriction(float) {}
void SurfaceMaterial::SetStaticFriction(float) {}
void SurfaceMaterial::SetRestitution(float) {}
void SurfaceMaterial::SetFrictionEnabled(bool) {}
void SurfaceMaterial::SetStrongFrictionEnabled(bool) {}
physx::PxMaterial *SurfaceMaterial::GetMaterial() {return m_material;}
void SurfaceMaterial::SetFrictionEnabled(bool b) {m_material->setFlag(physx::PxMaterialFlag::Enum::eDISABLE_FRICTION,!b);}
void SurfaceMaterial::SetStrongFrictionEnabled(bool b) {m_material->setFlag(physx::PxMaterialFlag::Enum::eDISABLE_STRONG_FRICTION,!b);}
float SurfaceMaterial::GetDynamicFriction() {return m_material->getDynamicFriction();}
float SurfaceMaterial::GetStaticFriction() {return m_material->getStaticFriction();}
void SurfaceMaterial::SetDynamicFriction(float friction) {m_material->setDynamicFriction(friction);}
void SurfaceMaterial::SetStaticFriction(float friction) {m_material->setStaticFriction(friction);}
void SurfaceMaterial::SetRestitution(float restitution) {m_material->setRestitution(restitution);}
#endif
const std::string &SurfaceMaterial::GetFootstepType() const {return m_footstepType;}
void SurfaceMaterial::SetFootstepType(const std::string &type) {m_footstepType = type;}

void SurfaceMaterial::SetNavigationFlags(pragma::nav::PolyFlags flags) {m_navigationFlags = flags;}
pragma::nav::PolyFlags SurfaceMaterial::GetNavigationFlags() const {return m_navigationFlags;}

std::ostream &operator<<(std::ostream &out,const SurfaceMaterial &surfaceMaterial)
{
	out<<"SurfaceMaterial["<<surfaceMaterial.GetIndex()<<"]["<<surfaceMaterial.GetIdentifier()<<"]";
	return out;
}
