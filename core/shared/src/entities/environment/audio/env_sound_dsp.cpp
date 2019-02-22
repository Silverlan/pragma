#include "stdafx_shared.h"
#include "pragma/entities/environment/audio/env_sound_dsp.h"
#include "pragma/entities/environment/audio/env_sound_dsp_chorus.h"
#include "pragma/entities/environment/audio/env_sound_dsp_distortion.h"
#include "pragma/entities/environment/audio/env_sound_dsp_eaxreverb.h"
#include "pragma/entities/environment/audio/env_sound_dsp_echo.h"
#include "pragma/entities/environment/audio/env_sound_dsp_equalizer.h"
#include "pragma/entities/environment/audio/env_sound_dsp_flanger.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <algorithm>

using namespace pragma;

void BaseEnvSoundDspComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(OnSetKeyValue(kvData.key,kvData.value) == true)
			return util::EventReply::Handled;
		return util::EventReply::Unhandled;
	});

	auto &ent = GetEntity();
	m_netEvSetGain = SetupNetEvent("set_gain");
	ent.AddComponent("toggle");
}

bool BaseEnvSoundDspComponent::OnSetKeyValue(const std::string &key,const std::string &val)
{
	if(ustring::compare(key,"dsp",false))
		m_kvDsp = val;
	else if(ustring::compare(key,"inner_radius",false))
		m_kvInnerRadius = util::to_float(val);
	else if(ustring::compare(key,"outer_radius",false))
		m_kvOuterRadius = util::to_float(val);
	else if(ustring::compare(key,"intensity",false))
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

void BaseEnvSoundDspComponent::SetDSPEffect(const std::string &identifier) {m_kvDsp = identifier;}
void BaseEnvSoundDspComponent::SetInnerRadius(float radius) {m_kvInnerRadius = radius;}
void BaseEnvSoundDspComponent::SetOuterRadius(float radius) {m_kvOuterRadius = radius;}

float BaseEnvSoundDspComponent::GetGain() const {return m_kvDspGain;}
void BaseEnvSoundDspComponent::SetGain(float gain) {m_kvDspGain = gain;}

bool BaseEnvSoundDspComponent::Input(const std::string &input,BaseEntity *activator,BaseEntity *caller,std::string data)
{
	if(input == "setgain")
		SetGain(ustring::to_int(data));
	else
		return false;
	return true;
}
