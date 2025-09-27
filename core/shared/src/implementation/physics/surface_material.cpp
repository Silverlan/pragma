// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include <pragma/engine.h>
#include "pragma/math/surfacematerial.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/phys_material.hpp"
#include "pragma/util/util_game.hpp"
#include <fsys/filesystem.h>
#include "datasystem.h"
#include <datasystem_color.h>
#include <datasystem_vector.h>
#include "pragma/ai/navsystem.h"
#include <algorithm>
#include <udm.hpp>

module pragma.shared;

import :math.surface_material;

SurfaceMaterialManager::SurfaceMaterialManager(pragma::physics::IEnvironment &env) : m_physEnv {env}
{
	auto &genericPhysMat = env.GetGenericMaterial();
	m_materials.push_back(SurfaceMaterial {m_physEnv, "generic", m_materials.size(), genericPhysMat});
	auto &surfMat = m_materials.back();
	surfMat.SetSoftImpactSound("fx.phys_impact_generic_soft");
	surfMat.SetHardImpactSound("fx.phys_impact_generic_hard");
	surfMat.SetBulletImpactSound("fx.phys_impact_generic_bullet");
	surfMat.SetImpactParticleEffect("impact_dust");

	genericPhysMat.SetSurfaceMaterial(surfMat);
}

bool SurfaceMaterialManager::Load(const std::string &path)
{
	std::string err;
	auto udmData = util::load_udm_asset(path, &err);
	if(udmData == nullptr)
		return false;
	auto &data = *udmData;
	auto udm = data.GetAssetData().GetData();
	for(auto pair : udm.ElIt()) {
		auto &identifier = pair.key;
		auto &physMat = Create(std::string {identifier});
		physMat.Load(pair.property);
	}
	return true;
}
SurfaceMaterial &SurfaceMaterialManager::Create(const std::string &identifier, Float staticFriction, Float dynamicFriction, Float restitution)
{
	auto *mat = GetMaterial(identifier);
	if(mat == nullptr) {
		auto physMat = m_physEnv.CreateMaterial(staticFriction, dynamicFriction, restitution);
		const auto numReserve = 50;
		m_materials.reserve((m_materials.size() / numReserve) * numReserve + numReserve);
		m_materials.push_back(SurfaceMaterial(m_physEnv, identifier, m_materials.size(), *physMat));
		mat = &m_materials.back();
	}
	return *mat;
}
SurfaceMaterial &SurfaceMaterialManager::Create(const std::string &identifier, Float friction, Float restitution) { return Create(identifier, friction, friction, restitution); }
SurfaceMaterial *SurfaceMaterialManager::GetMaterial(const std::string &id)
{
	auto it = std::find_if(m_materials.begin(), m_materials.end(), [&id](const SurfaceMaterial &mat) { return (mat.GetIdentifier() == id) ? true : false; });
	if(it == m_materials.end())
		return nullptr;
	return &(*it);
}
std::vector<SurfaceMaterial> &SurfaceMaterialManager::GetMaterials() { return m_materials; }

////////////////////////////////////

SurfaceMaterial::SurfaceMaterial(const SurfaceMaterial &other) : m_physEnv {other.m_physEnv}, m_navigationFlags(pragma::nav::PolyFlags::Walk)
{
	m_index = other.m_index;
	m_identifier = other.m_identifier;
	m_footstepType = other.m_footstepType;
	m_softImpactSound = other.m_softImpactSound;
	m_hardImpactSound = other.m_hardImpactSound;
	m_bulletImpactSound = other.m_bulletImpactSound;
	m_impactParticle = other.m_impactParticle;
	m_navigationFlags = other.m_navigationFlags;
	m_physMaterial = other.m_physMaterial;
	m_surfaceType = other.m_surfaceType;
	if(other.m_liquidInfo != nullptr) {
		m_liquidInfo = std::make_unique<PhysLiquid>();
		*m_liquidInfo = *other.m_liquidInfo;
	}
	m_audioInfo = other.m_audioInfo;
	m_pbrInfo = other.m_pbrInfo;
}

SurfaceMaterial::SurfaceMaterial(pragma::physics::IEnvironment &env, const std::string &identifier, UInt idx, pragma::physics::IMaterial &physMat)
    : m_physEnv {env}, m_physMaterial {std::static_pointer_cast<pragma::physics::IMaterial>(physMat.shared_from_this())}, m_index(idx), m_identifier(identifier), m_footstepType("fx.fst_concrete")
{
	physMat.SetSurfaceMaterial(*this);
}

void SurfaceMaterial::SetSoftImpactSound(const std::string &snd) { m_softImpactSound = snd; }
const std::string &SurfaceMaterial::GetSoftImpactSound() const { return m_softImpactSound; }
void SurfaceMaterial::SetHardImpactSound(const std::string &snd) { m_hardImpactSound = snd; }
const std::string &SurfaceMaterial::GetHardImpactSound() const { return m_hardImpactSound; }
void SurfaceMaterial::SetBulletImpactSound(const std::string &snd) { m_bulletImpactSound = snd; }
const std::string &SurfaceMaterial::GetBulletImpactSound() const { return m_bulletImpactSound; }
void SurfaceMaterial::SetImpactParticleEffect(const std::string &particle) { m_impactParticle = particle; }
const std::string &SurfaceMaterial::GetImpactParticleEffect() const { return m_impactParticle; }

PhysLiquid &SurfaceMaterial::InitializeLiquid()
{
	if(m_liquidInfo == nullptr)
		m_liquidInfo = std::make_unique<PhysLiquid>();
	return *m_liquidInfo;
}
void SurfaceMaterial::SetDensity(float density) { InitializeLiquid().density = density; }
float SurfaceMaterial::GetDensity() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_DENSITY;
	return m_liquidInfo->density;
}
void SurfaceMaterial::SetLinearDragCoefficient(float coefficient) { InitializeLiquid().linearDragCoefficient = coefficient; }
float SurfaceMaterial::GetLinearDragCoefficient() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_LINEAR_DRAG_COEFFICIENT;
	return m_liquidInfo->linearDragCoefficient;
}
void SurfaceMaterial::SetTorqueDragCoefficient(float coefficient) { InitializeLiquid().torqueDragCoefficient = coefficient; }
float SurfaceMaterial::GetTorqueDragCoefficient() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_TORQUE_DRAG_COEFFICIENT;
	return m_liquidInfo->torqueDragCoefficient;
}
void SurfaceMaterial::SetWaveStiffness(float stiffness) { InitializeLiquid().stiffness = stiffness; }
float SurfaceMaterial::GetWaveStiffness() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_STIFFNESS;
	return m_liquidInfo->stiffness;
}
void SurfaceMaterial::SetWavePropagation(float propagation) { InitializeLiquid().propagation = propagation; }
float SurfaceMaterial::GetWavePropagation() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_PROPAGATION;
	return m_liquidInfo->propagation;
}

const SurfaceMaterial::PBRInfo &SurfaceMaterial::GetPBRInfo() const { return const_cast<SurfaceMaterial *>(this)->GetPBRInfo(); }
SurfaceMaterial::PBRInfo &SurfaceMaterial::GetPBRInfo() { return m_pbrInfo; }
void SurfaceMaterial::SetPBRInfo(const PBRInfo &pbrInfo) { m_pbrInfo = pbrInfo; }
const SurfaceMaterial::AudioInfo &SurfaceMaterial::GetAudioInfo() const { return m_audioInfo; }
void SurfaceMaterial::SetAudioInfo(const AudioInfo &info) { m_audioInfo = info; }
void SurfaceMaterial::SetAudioLowFrequencyAbsorption(float absp) { m_audioInfo.lowFreqAbsorption = absp; }
float SurfaceMaterial::GetAudioLowFrequencyAbsorption() const { return m_audioInfo.lowFreqAbsorption; }
void SurfaceMaterial::SetAudioMidFrequencyAbsorption(float absp) { m_audioInfo.midFreqAbsorption = absp; }
float SurfaceMaterial::GetAudioMidFrequencyAbsorption() const { return m_audioInfo.midFreqAbsorption; }
void SurfaceMaterial::SetAudioHighFrequencyAbsorption(float absp) { m_audioInfo.highFreqAbsorption = absp; }
float SurfaceMaterial::GetAudioHighFrequencyAbsorption() const { return m_audioInfo.highFreqAbsorption; }
void SurfaceMaterial::SetAudioScattering(float scattering) { m_audioInfo.scattering = scattering; }
float SurfaceMaterial::GetAudioScattering() const { return m_audioInfo.scattering; }
void SurfaceMaterial::SetAudioLowFrequencyTransmission(float transmission) { m_audioInfo.lowFreqTransmission = transmission; }
float SurfaceMaterial::GetAudioLowFrequencyTransmission() const { return m_audioInfo.lowFreqTransmission; }
void SurfaceMaterial::SetAudioMidFrequencyTransmission(float transmission) { m_audioInfo.midFreqTransmission = transmission; }
float SurfaceMaterial::GetAudioMidFrequencyTransmission() const { return m_audioInfo.midFreqTransmission; }
void SurfaceMaterial::SetAudioHighFrequencyTransmission(float transmission) { m_audioInfo.highFreqTransmission = transmission; }
float SurfaceMaterial::GetAudioHighFrequencyTransmission() const { return m_audioInfo.highFreqTransmission; }

void SurfaceMaterial::Load(udm::LinkedPropertyWrapper &prop)
{
	if(prop["navigation_flags"]) {
		auto flags = pragma::nav::PolyFlags::None;
		udm::read_flag(prop["navigation_flags"], flags, pragma::nav::PolyFlags::Walk, "walk");
		udm::read_flag(prop["navigation_flags"], flags, pragma::nav::PolyFlags::Swim, "swim");
		udm::read_flag(prop["navigation_flags"], flags, pragma::nav::PolyFlags::Door, "door");
		udm::read_flag(prop["navigation_flags"], flags, pragma::nav::PolyFlags::Jump, "jump");
		udm::read_flag(prop["navigation_flags"], flags, pragma::nav::PolyFlags::Disabled, "disabled");
		udm::read_flag(prop["navigation_flags"], flags, pragma::nav::PolyFlags::All, "all");
		SetNavigationFlags(flags);
	}
	prop["footsteps"](m_footstepType);
	prop["impact_bullet"](m_bulletImpactSound);
	prop["impact_effect"](m_impactParticle);
	prop["impact_soft"](m_softImpactSound);
	prop["impact_hard"](m_hardImpactSound);
	std::string surfType;
	prop["surface_type"](surfType);
	SetSurfaceType(surfType);

	if(prop["density"])
		prop["density"](InitializeLiquid().density);
	if(prop["linear_drag_coefficient"])
		prop["linear_drag_coefficient"](InitializeLiquid().linearDragCoefficient);
	if(prop["torque_drag_coefficient"])
		prop["torque_drag_coefficient"](InitializeLiquid().torqueDragCoefficient);
	if(prop["wave_stiffness"])
		prop["wave_stiffness"](InitializeLiquid().stiffness);
	if(prop["wave_propagation"])
		prop["wave_propagation"](InitializeLiquid().propagation);
	if(prop["friction"]) {
		auto friction = 0.f;
		prop["friction"](friction);
		SetStaticFriction(friction);
		SetDynamicFriction(friction);
	}
	if(prop["static_friction"]) {
		auto friction = 0.f;
		prop["static_friction"](friction);
		SetStaticFriction(friction);
	}
	if(prop["dynamic_friction"]) {
		auto friction = 0.f;
		prop["dynamic_friction"](friction);
		SetDynamicFriction(friction);
	}
	if(prop["restitution"]) {
		auto restitution = 0.f;
		prop["restitution"](restitution);
		SetRestitution(restitution);
	}

	auto udmAudio = prop["audio"];
	if(udmAudio) {
		udmAudio["low_frequency_absorption"](m_audioInfo.lowFreqAbsorption);
		udmAudio["mid_frequency_absorption"](m_audioInfo.midFreqAbsorption);
		udmAudio["high_frequency_absorption"](m_audioInfo.highFreqAbsorption);
		udmAudio["scattering"](m_audioInfo.scattering);
		udmAudio["low_frequency_transmission"](m_audioInfo.lowFreqTransmission);
		udmAudio["mid_frequency_transmission"](m_audioInfo.midFreqTransmission);
		udmAudio["high_frequency_transmission"](m_audioInfo.highFreqTransmission);
	}

	if(prop["ior"]) {
		m_ior = float {};
		prop["ior"](*m_ior);
	}

	auto udmPbr = prop["pbr"];
	if(udmPbr) {
		udmPbr["metalness"](GetPBRInfo().metalness);
		udmPbr["roughness"](GetPBRInfo().roughness);

		auto udmSss = udmPbr["subsurface_scattering"];
		if(udmSss) {
			udmSss["factor"](GetPBRInfo().subsurface.factor);
			if(udmSss["color"]) {
				udm::Srgba col {};
				udmSss["color"](col);
				GetPBRInfo().subsurface.color = {col[0], col[1], col[2], col[3]};
			}
			udmSss["scatter_color"](GetPBRInfo().subsurface.scatterColor);
			udmSss["radius_mm"](GetPBRInfo().subsurface.radiusMM);
		}
	}
}

void SurfaceMaterial::Reset()
{
	m_footstepType = "generic";

	m_audioInfo = {};
	m_navigationFlags = pragma::nav::PolyFlags::None;

	m_physMaterial->SetFriction(0.5f);
	m_physMaterial->SetRestitution(0.5f);
}

pragma::physics::IMaterial &SurfaceMaterial::GetPhysicsMaterial() const { return *m_physMaterial; }
pragma::physics::SurfaceType *SurfaceMaterial::GetSurfaceType() const { return m_surfaceType.Get(); }
void SurfaceMaterial::SetSurfaceType(const std::string &surfaceType) { m_surfaceType = m_physEnv.GetSurfaceTypeManager().RegisterType(surfaceType); }
const std::string &SurfaceMaterial::GetIdentifier() const { return m_identifier; }
void SurfaceMaterial::SetFriction(Float friction)
{
	SetStaticFriction(friction);
	SetDynamicFriction(friction);
}
const std::optional<float> &SurfaceMaterial::GetIOR() const { return m_ior; }
void SurfaceMaterial::SetIOR(float ior) { m_ior = ior; }
void SurfaceMaterial::ClearIOR() { m_ior = {}; }
Float SurfaceMaterial::GetRestitution() const { return m_physMaterial->GetRestitution(); }
void SurfaceMaterial::SetRestitution(Float restitution) { m_physMaterial->SetRestitution(restitution); }
float SurfaceMaterial::GetStaticFriction() const { return m_physMaterial->GetStaticFriction(); }
void SurfaceMaterial::SetStaticFriction(float friction) { m_physMaterial->SetStaticFriction(friction); }
float SurfaceMaterial::GetDynamicFriction() const { return m_physMaterial->GetDynamicFriction(); }
void SurfaceMaterial::SetDynamicFriction(float friction) { m_physMaterial->SetDynamicFriction(friction); }
UInt SurfaceMaterial::GetIndex() const { return m_index; }

const std::string &SurfaceMaterial::GetFootstepType() const { return m_footstepType; }
void SurfaceMaterial::SetFootstepType(const std::string &type) { m_footstepType = type; }

void SurfaceMaterial::SetNavigationFlags(pragma::nav::PolyFlags flags) { m_navigationFlags = flags; }
pragma::nav::PolyFlags SurfaceMaterial::GetNavigationFlags() const { return m_navigationFlags; }

std::ostream &operator<<(std::ostream &out, const SurfaceMaterial &surfaceMaterial)
{
	out << "SurfaceMaterial[" << surfaceMaterial.GetIndex() << "][" << surfaceMaterial.GetIdentifier() << "]";
	return out;
}
