// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.surface_material;

pragma::physics::SurfaceMaterialManager::SurfaceMaterialManager(IEnvironment &env) : m_physEnv {env}
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

bool pragma::physics::SurfaceMaterialManager::Load(const std::string &path)
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
pragma::physics::SurfaceMaterial &pragma::physics::SurfaceMaterialManager::Create(const std::string &identifier, Float staticFriction, Float dynamicFriction, Float restitution)
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
pragma::physics::SurfaceMaterial &pragma::physics::SurfaceMaterialManager::Create(const std::string &identifier, Float friction, Float restitution) { return Create(identifier, friction, friction, restitution); }
pragma::physics::SurfaceMaterial *pragma::physics::SurfaceMaterialManager::GetMaterial(const std::string &id)
{
	auto it = std::find_if(m_materials.begin(), m_materials.end(), [&id](const SurfaceMaterial &mat) { return (mat.GetIdentifier() == id) ? true : false; });
	if(it == m_materials.end())
		return nullptr;
	return &(*it);
}
std::vector<pragma::physics::SurfaceMaterial> &pragma::physics::SurfaceMaterialManager::GetMaterials() { return m_materials; }

////////////////////////////////////

pragma::physics::SurfaceMaterial::SurfaceMaterial(const SurfaceMaterial &other) : m_physEnv {other.m_physEnv}, m_navigationFlags(nav::PolyFlags::Walk)
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

pragma::physics::SurfaceMaterial::SurfaceMaterial(IEnvironment &env, const std::string &identifier, UInt idx, IMaterial &physMat)
    : m_physEnv {env}, m_physMaterial {std::static_pointer_cast<IMaterial>(physMat.shared_from_this())}, m_index(idx), m_identifier(identifier), m_footstepType("fx.fst_concrete")
{
	physMat.SetSurfaceMaterial(*this);
}

void pragma::physics::SurfaceMaterial::SetSoftImpactSound(const std::string &snd) { m_softImpactSound = snd; }
const std::string &pragma::physics::SurfaceMaterial::GetSoftImpactSound() const { return m_softImpactSound; }
void pragma::physics::SurfaceMaterial::SetHardImpactSound(const std::string &snd) { m_hardImpactSound = snd; }
const std::string &pragma::physics::SurfaceMaterial::GetHardImpactSound() const { return m_hardImpactSound; }
void pragma::physics::SurfaceMaterial::SetBulletImpactSound(const std::string &snd) { m_bulletImpactSound = snd; }
const std::string &pragma::physics::SurfaceMaterial::GetBulletImpactSound() const { return m_bulletImpactSound; }
void pragma::physics::SurfaceMaterial::SetImpactParticleEffect(const std::string &particle) { m_impactParticle = particle; }
const std::string &pragma::physics::SurfaceMaterial::GetImpactParticleEffect() const { return m_impactParticle; }

pragma::physics::PhysLiquid &pragma::physics::SurfaceMaterial::InitializeLiquid()
{
	if(m_liquidInfo == nullptr)
		m_liquidInfo = std::make_unique<PhysLiquid>();
	return *m_liquidInfo;
}
void pragma::physics::SurfaceMaterial::SetDensity(float density) { InitializeLiquid().density = density; }
float pragma::physics::SurfaceMaterial::GetDensity() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_DENSITY;
	return m_liquidInfo->density;
}
void pragma::physics::SurfaceMaterial::SetLinearDragCoefficient(float coefficient) { InitializeLiquid().linearDragCoefficient = coefficient; }
float pragma::physics::SurfaceMaterial::GetLinearDragCoefficient() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_LINEAR_DRAG_COEFFICIENT;
	return m_liquidInfo->linearDragCoefficient;
}
void pragma::physics::SurfaceMaterial::SetTorqueDragCoefficient(float coefficient) { InitializeLiquid().torqueDragCoefficient = coefficient; }
float pragma::physics::SurfaceMaterial::GetTorqueDragCoefficient() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_TORQUE_DRAG_COEFFICIENT;
	return m_liquidInfo->torqueDragCoefficient;
}
void pragma::physics::SurfaceMaterial::SetWaveStiffness(float stiffness) { InitializeLiquid().stiffness = stiffness; }
float pragma::physics::SurfaceMaterial::GetWaveStiffness() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_STIFFNESS;
	return m_liquidInfo->stiffness;
}
void pragma::physics::SurfaceMaterial::SetWavePropagation(float propagation) { InitializeLiquid().propagation = propagation; }
float pragma::physics::SurfaceMaterial::GetWavePropagation() const
{
	if(m_liquidInfo == nullptr)
		return PHYS_LIQUID_DEFAULT_PROPAGATION;
	return m_liquidInfo->propagation;
}

const pragma::physics::SurfaceMaterial::PBRInfo &pragma::physics::SurfaceMaterial::GetPBRInfo() const { return const_cast<SurfaceMaterial *>(this)->GetPBRInfo(); }
pragma::physics::SurfaceMaterial::PBRInfo &pragma::physics::SurfaceMaterial::GetPBRInfo() { return m_pbrInfo; }
void pragma::physics::SurfaceMaterial::SetPBRInfo(const PBRInfo &pbrInfo) { m_pbrInfo = pbrInfo; }
const pragma::physics::SurfaceMaterial::AudioInfo &pragma::physics::SurfaceMaterial::GetAudioInfo() const { return m_audioInfo; }
void pragma::physics::SurfaceMaterial::SetAudioInfo(const AudioInfo &info) { m_audioInfo = info; }
void pragma::physics::SurfaceMaterial::SetAudioLowFrequencyAbsorption(float absp) { m_audioInfo.lowFreqAbsorption = absp; }
float pragma::physics::SurfaceMaterial::GetAudioLowFrequencyAbsorption() const { return m_audioInfo.lowFreqAbsorption; }
void pragma::physics::SurfaceMaterial::SetAudioMidFrequencyAbsorption(float absp) { m_audioInfo.midFreqAbsorption = absp; }
float pragma::physics::SurfaceMaterial::GetAudioMidFrequencyAbsorption() const { return m_audioInfo.midFreqAbsorption; }
void pragma::physics::SurfaceMaterial::SetAudioHighFrequencyAbsorption(float absp) { m_audioInfo.highFreqAbsorption = absp; }
float pragma::physics::SurfaceMaterial::GetAudioHighFrequencyAbsorption() const { return m_audioInfo.highFreqAbsorption; }
void pragma::physics::SurfaceMaterial::SetAudioScattering(float scattering) { m_audioInfo.scattering = scattering; }
float pragma::physics::SurfaceMaterial::GetAudioScattering() const { return m_audioInfo.scattering; }
void pragma::physics::SurfaceMaterial::SetAudioLowFrequencyTransmission(float transmission) { m_audioInfo.lowFreqTransmission = transmission; }
float pragma::physics::SurfaceMaterial::GetAudioLowFrequencyTransmission() const { return m_audioInfo.lowFreqTransmission; }
void pragma::physics::SurfaceMaterial::SetAudioMidFrequencyTransmission(float transmission) { m_audioInfo.midFreqTransmission = transmission; }
float pragma::physics::SurfaceMaterial::GetAudioMidFrequencyTransmission() const { return m_audioInfo.midFreqTransmission; }
void pragma::physics::SurfaceMaterial::SetAudioHighFrequencyTransmission(float transmission) { m_audioInfo.highFreqTransmission = transmission; }
float pragma::physics::SurfaceMaterial::GetAudioHighFrequencyTransmission() const { return m_audioInfo.highFreqTransmission; }

void pragma::physics::SurfaceMaterial::Load(udm::LinkedPropertyWrapper &prop)
{
	if(prop["navigation_flags"]) {
		auto flags = nav::PolyFlags::None;
		udm::read_flag(prop["navigation_flags"], flags, nav::PolyFlags::Walk, "walk");
		udm::read_flag(prop["navigation_flags"], flags, nav::PolyFlags::Swim, "swim");
		udm::read_flag(prop["navigation_flags"], flags, nav::PolyFlags::Door, "door");
		udm::read_flag(prop["navigation_flags"], flags, nav::PolyFlags::Jump, "jump");
		udm::read_flag(prop["navigation_flags"], flags, nav::PolyFlags::Disabled, "disabled");
		udm::read_flag(prop["navigation_flags"], flags, nav::PolyFlags::All, "all");
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

void pragma::physics::SurfaceMaterial::Reset()
{
	m_footstepType = "generic";

	m_audioInfo = {};
	m_navigationFlags = nav::PolyFlags::None;

	m_physMaterial->SetFriction(0.5f);
	m_physMaterial->SetRestitution(0.5f);
}

pragma::physics::IMaterial &pragma::physics::SurfaceMaterial::GetPhysicsMaterial() const { return *m_physMaterial; }
pragma::physics::SurfaceType *pragma::physics::SurfaceMaterial::GetSurfaceType() const { return m_surfaceType.Get(); }
void pragma::physics::SurfaceMaterial::SetSurfaceType(const std::string &surfaceType) { m_surfaceType = m_physEnv.GetSurfaceTypeManager().RegisterType(surfaceType); }
const std::string &pragma::physics::SurfaceMaterial::GetIdentifier() const { return m_identifier; }
void pragma::physics::SurfaceMaterial::SetFriction(Float friction)
{
	SetStaticFriction(friction);
	SetDynamicFriction(friction);
}
const std::optional<float> &pragma::physics::SurfaceMaterial::GetIOR() const { return m_ior; }
void pragma::physics::SurfaceMaterial::SetIOR(float ior) { m_ior = ior; }
void pragma::physics::SurfaceMaterial::ClearIOR() { m_ior = {}; }
Float pragma::physics::SurfaceMaterial::GetRestitution() const { return m_physMaterial->GetRestitution(); }
void pragma::physics::SurfaceMaterial::SetRestitution(Float restitution) { m_physMaterial->SetRestitution(restitution); }
float pragma::physics::SurfaceMaterial::GetStaticFriction() const { return m_physMaterial->GetStaticFriction(); }
void pragma::physics::SurfaceMaterial::SetStaticFriction(float friction) { m_physMaterial->SetStaticFriction(friction); }
float pragma::physics::SurfaceMaterial::GetDynamicFriction() const { return m_physMaterial->GetDynamicFriction(); }
void pragma::physics::SurfaceMaterial::SetDynamicFriction(float friction) { m_physMaterial->SetDynamicFriction(friction); }
UInt pragma::physics::SurfaceMaterial::GetIndex() const { return m_index; }

const std::string &pragma::physics::SurfaceMaterial::GetFootstepType() const { return m_footstepType; }
void pragma::physics::SurfaceMaterial::SetFootstepType(const std::string &type) { m_footstepType = type; }

void pragma::physics::SurfaceMaterial::SetNavigationFlags(nav::PolyFlags flags) { m_navigationFlags = flags; }
pragma::nav::PolyFlags pragma::physics::SurfaceMaterial::GetNavigationFlags() const { return m_navigationFlags; }

std::ostream &operator<<(std::ostream &out, const pragma::physics::SurfaceMaterial &surfaceMaterial)
{
	out << "SurfaceMaterial[" << surfaceMaterial.GetIndex() << "][" << surfaceMaterial.GetIdentifier() << "]";
	return out;
}
