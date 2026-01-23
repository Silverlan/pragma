// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.audio.dsp.base_dsp;

using namespace pragma;

void BaseEnvSoundDspComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(OnSetKeyValue(kvData.key, kvData.value) == true)
			return util::EventReply::Handled;
		return util::EventReply::Unhandled;
	});

	auto &ent = GetEntity();
	m_netEvSetGain = SetupNetEvent("set_gain");
	ent.AddComponent("toggle");
}

bool BaseEnvSoundDspComponent::OnSetKeyValue(const std::string &key, const std::string &val)
{
	if(pragma::string::compare<std::string>(key, "dsp", false))
		m_kvDsp = val;
	else if(pragma::string::compare<std::string>(key, "inner_radius", false))
		m_kvInnerRadius = util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "outer_radius", false))
		m_kvOuterRadius = util::to_float(val);
	else if(pragma::string::compare<std::string>(key, "intensity", false))
		m_kvDspGain = util::to_float(val);
	else
		return false;
	return true;
}

BaseEnvSoundDspChorus::BaseEnvSoundDspChorus() {}
BaseEnvSoundDspDistortion::BaseEnvSoundDspDistortion() {}
BaseEnvSoundDspEAXReverb::BaseEnvSoundDspEAXReverb() {}
BaseEnvSoundDspEcho::BaseEnvSoundDspEcho() {}
BaseEnvSoundDspEqualizer::BaseEnvSoundDspEqualizer() {}
BaseEnvSoundDspFlanger::BaseEnvSoundDspFlanger() {}

void BaseEnvSoundDspComponent::SetDSPEffect(const std::string &identifier) { m_kvDsp = identifier; }
void BaseEnvSoundDspComponent::SetInnerRadius(float radius) { m_kvInnerRadius = radius; }
void BaseEnvSoundDspComponent::SetOuterRadius(float radius) { m_kvOuterRadius = radius; }

float BaseEnvSoundDspComponent::GetGain() const { return m_kvDspGain; }
void BaseEnvSoundDspComponent::SetGain(float gain) { m_kvDspGain = gain; }

bool BaseEnvSoundDspComponent::Input(const std::string &input, ecs::BaseEntity *activator, ecs::BaseEntity *caller, std::string data)
{
	if(input == "setgain")
		SetGain(string::to_int(data));
	else
		return false;
	return true;
}
