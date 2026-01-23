// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.audio.sound_probe;
import :client_state;

using namespace pragma;

decltype(CEnvSoundProbeComponent::s_probes) CEnvSoundProbeComponent::s_probes = {};
decltype(CEnvSoundProbeComponent::s_probeCallback) CEnvSoundProbeComponent::s_probeCallback = {};
void CEnvSoundProbeComponent::ClearProbes() { s_probes = {}; }
const std::vector<CEnvSoundProbeComponent::ProbeInfo> &CEnvSoundProbeComponent::GetProbes() { return s_probes; }

void CEnvSoundProbeComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "spacing", false))
			m_spacing = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "height_above_floor", false))
			m_heightAboveFloor = util::to_float(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "radius", false))
			m_radius = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}
void CEnvSoundProbeComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(s_probes.size() == s_probes.capacity())
		s_probes.reserve(s_probes.size() + 50);

	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	auto &origin = pTrComponent->GetPosition();
	auto mdlComponent = ent.GetModelComponent();
	if(!mdlComponent || mdlComponent->GetModel() == nullptr) {
		// Use radius
		s_probes.push_back({origin, origin, Placement::Centroid, m_radius, 0.f});
		return;
	}
	auto pPhysComponent = ent.GetPhysicsComponent();
	Vector3 min {};
	Vector3 max {};
	if(pPhysComponent != nullptr)
		pPhysComponent->GetCollisionBounds(&min, &max);
	s_probes.push_back({origin + min, origin + max, Placement::UniformFloor, m_spacing, m_heightAboveFloor});
	if(s_probeCallback.IsValid() == false) {
		s_probeCallback = get_client_state()->AddCallback("EndGame", FunctionCallback<void, CGame *>::Create([](CGame *game) {
			ClearProbes();
			s_probeCallback.Remove();
		}));
	}
	ent.RemoveSafely();
}
void CEnvSoundProbeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////

void CEnvSoundProbe::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CEnvSoundProbeComponent>();
}
