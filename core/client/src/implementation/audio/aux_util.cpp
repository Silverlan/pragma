// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :audio.aux_util;
import :engine;

static uint32_t s_globalEffectId = std::numeric_limits<uint32_t>::max();
static void debug_audio_aux_effect(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr) {
		Con::cwar << "Sound engine hasn't been initialized!" << Con::endl;
		return;
	}
	if(argv.empty() == true) {
		if(s_globalEffectId != std::numeric_limits<uint32_t>::max()) {
			soundSys->RemoveGlobalEffect(s_globalEffectId);
			return;
		}
		Con::cwar << "No effect has been specified!" << Con::endl;
		return;
	}
	auto &dspName = argv.front();
	auto effect = pragma::get_cengine()->GetAuxEffect(dspName);
	if(effect == nullptr) {
		Con::cwar << "No auxiliary effect found with name '" << dspName << "'!" << Con::endl;
		return;
	}
	s_globalEffectId = soundSys->AddGlobalEffect(*effect); // TODO: Gain
	if(s_globalEffectId == std::numeric_limits<uint32_t>::max())
		Con::cwar << "Unable to apply auxiliary effect '" << dspName << "'!" << Con::endl;
	else
		Con::cout << "Auxiliary effect '" << dspName << "' has been applied!" << Con::endl;
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_audio_aux_effect", &debug_audio_aux_effect, pragma::console::ConVarFlags::None, "Applies a global DSP effect. Usage: debug_audio_aux_effect <dspName> <gain>");
}

const std::vector<std::string> &al::get_aux_types()
{
	static std::vector<std::string> types = {"reverb", "chorus", "distortion", "echo", "flanger", "frequency_shifter", "vocal_morpher", "pitch_shifter", "ring_modulator", "autowah", "compressor", "equalizer", "eaxreverb"};
	return types;
}

namespace al {
	static std::shared_ptr<al::IEffect> create_aux_effect(const std::string *name, const std::string &type, udm::LinkedPropertyWrapper &prop);
};

std::shared_ptr<al::IEffect> al::create_aux_effect(const std::string *name, const std::string &type, udm::LinkedPropertyWrapper &prop)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return nullptr;
	if(type == "reverb") {
		al::EfxEaxReverbProperties props {};
		prop["density"](props.flDensity);
		prop["diffusion"](props.flDiffusion);
		prop["gain"](props.flGain);
		prop["gainhf"](props.flGainHF);
		prop["gainlf"](props.flGainLF);
		prop["decay_time"](props.flDecayTime);
		prop["decay_hfratio"](props.flDecayHFRatio);
		prop["reflections_gain"](props.flReflectionsGain);
		prop["reflections_delay"](props.flReflectionsDelay);
		prop["late_reverb_gain"](props.flLateReverbGain);
		prop["late_reverb_delay"](props.flLateReverbDelay);
		prop["air_absorption_gainhf"](props.flAirAbsorptionGainHF);
		prop["room_rolloff_factor"](props.flRoomRolloffFactor);
		prop["decay_hflimit"](props.iDecayHFLimit);
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "chorus") {
		al::EfxChorusProperties props {};
		prop["rate"](props.flRate);
		prop["depth"](props.flDepth);
		prop["feedback"](props.flFeedback);
		prop["delay"](props.flDelay);

		prop["waveform"](props.iWaveform);
		prop["phase"](props.iPhase);
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "distortion") {
		al::EfxDistortionProperties props {};
		prop["edge"](props.flEdge);
		prop["gain"](props.flGain);
		prop["lowpass_cutoff"](props.flLowpassCutoff);
		prop["eqcenter"](props.flEQCenter);
		prop["eqbandwidth"](props.flEQBandwidth);
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "echo") {
		al::EfxEchoProperties props {};
		prop["delay"](props.flDelay);
		prop["lrdelay"](props.flLRDelay);
		prop["damping"](props.flDamping);
		prop["feedback"](props.flFeedback);
		prop["spread"](props.flSpread);
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "flanger") {
		al::EfxFlangerProperties props {};
		prop["rate"](props.flRate);
		prop["depth"](props.flDepth);
		prop["feedback"](props.flFeedback);
		prop["delay"](props.flDelay);

		prop["waveform"](props.iWaveform);
		prop["phase"](props.iPhase);
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "frequency_shifter") {
		al::EfxFrequencyShifterProperties props {};
		prop["frequency"](props.flFrequency);

		prop["left_direction"](props.iLeftDirection);
		prop["right_direction"](props.iRightDirection);
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "vocal_morpher") {
		al::EfxVocalMorpherProperties props {};
		prop["rate"](props.flRate);

		prop["phonemea_coarse_tuning"](props.iPhonemeACoarseTuning);
		prop["phonemeb_coarse_tuning"](props.iPhonemeBCoarseTuning);
		prop["waveform"](props.iWaveform);

		const std::unordered_map<std::string, int32_t> phonemes = {{"a", umath::to_integral(al::VocalMorpherPhoneme::A)}, {"e", umath::to_integral(al::VocalMorpherPhoneme::E)}, {"i", umath::to_integral(al::VocalMorpherPhoneme::I)}, {"o", umath::to_integral(al::VocalMorpherPhoneme::O)},
		  {"u", umath::to_integral(al::VocalMorpherPhoneme::U)}, {"aa", umath::to_integral(al::VocalMorpherPhoneme::AA)}, {"ae", umath::to_integral(al::VocalMorpherPhoneme::AE)}, {"ah", umath::to_integral(al::VocalMorpherPhoneme::AH)},
		  {"ao", umath::to_integral(al::VocalMorpherPhoneme::AO)}, {"eh", umath::to_integral(al::VocalMorpherPhoneme::EH)}, {"er", umath::to_integral(al::VocalMorpherPhoneme::ER)}, {"ih", umath::to_integral(al::VocalMorpherPhoneme::IH)},
		  {"iy", umath::to_integral(al::VocalMorpherPhoneme::IY)}, {"uh", umath::to_integral(al::VocalMorpherPhoneme::UH)}, {"uw", umath::to_integral(al::VocalMorpherPhoneme::UW)}, {"b", umath::to_integral(al::VocalMorpherPhoneme::B)}, {"d", umath::to_integral(al::VocalMorpherPhoneme::D)},
		  {"f", umath::to_integral(al::VocalMorpherPhoneme::F)}, {"g", umath::to_integral(al::VocalMorpherPhoneme::G)}, {"j", umath::to_integral(al::VocalMorpherPhoneme::J)}, {"k", umath::to_integral(al::VocalMorpherPhoneme::K)}, {"l", umath::to_integral(al::VocalMorpherPhoneme::L)},
		  {"m", umath::to_integral(al::VocalMorpherPhoneme::M)}, {"n", umath::to_integral(al::VocalMorpherPhoneme::N)}, {"p", umath::to_integral(al::VocalMorpherPhoneme::P)}, {"r", umath::to_integral(al::VocalMorpherPhoneme::R)}, {"s", umath::to_integral(al::VocalMorpherPhoneme::S)},
		  {"t", umath::to_integral(al::VocalMorpherPhoneme::T)}, {"v", umath::to_integral(al::VocalMorpherPhoneme::V)}, {"z", umath::to_integral(al::VocalMorpherPhoneme::Z)}};

		const std::unordered_map<std::string, int32_t *> phonemeKeys = {{"phonemea", &props.iPhonemeA}, {"phonemeb", &props.iPhonemeB}};
		for(auto &pair : phonemeKeys) {
			auto &key = pair.first;
			std::string val;
			prop[key](val);
			if(!val.empty()) {
				ustring::to_lower(val);
				auto itPhoneme = phonemes.find(val);
				if(itPhoneme != phonemes.end())
					*pair.second = itPhoneme->second;
			}
			else
				prop[key](*pair.second);
		}
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "pitch_shifter") {
		al::EfxPitchShifterProperties props {};
		prop["coarse_tune"](props.iCoarseTune);
		prop["fine_tune"](props.iFineTune);
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "ring_modulator") {
		al::EfxRingModulatorProperties props {};
		prop["frequency"](props.flFrequency);
		prop["highpass_cutoff"](props.flHighpassCutoff);

		prop["waveform"](props.iWaveform);
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "autowah") {
		al::EfxAutoWahProperties props {};
		prop["attack_time"](props.flAttackTime);
		prop["release_time"](props.flReleaseTime);
		prop["resonance"](props.flResonance);
		prop["peak_gain"](props.flPeakGain);
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "compressor") {
		al::EfxCompressor props {};
		prop["onoff"](props.iOnOff);
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "equalizer") {
		al::EfxEqualizer props {};
		prop["low_gain"](props.flLowGain);
		prop["low_cutoff"](props.flLowCutoff);
		prop["mid1_gain"](props.flMid1Gain);
		prop["mid1_center"](props.flMid1Center);
		prop["mid1_width"](props.flMid1Width);
		prop["mid2_gain"](props.flMid2Gain);
		prop["mid2_center"](props.flMid2Center);
		prop["mid2_width"](props.flMid2Width);
		prop["high_gain"](props.flHighGain);
		prop["high_cutoff"](props.flHighCutoff);
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "eaxreverb") {
		al::EfxEaxReverbProperties props {};
		prop["density"](props.flDensity);
		prop["diffusion"](props.flDiffusion);
		prop["gain"](props.flGain);
		prop["gainhf"](props.flGainHF);
		prop["gainlf"](props.flGainLF);
		prop["decay_time"](props.flDecayTime);
		prop["decay_hfratio"](props.flDecayHFRatio);
		prop["decay_lfratio"](props.flDecayLFRatio);
		prop["reflections_gain"](props.flReflectionsGain);
		prop["reflections_delay"](props.flReflectionsDelay);
		prop["late_reverb_gain"](props.flLateReverbGain);
		prop["late_reverb_delay"](props.flLateReverbDelay);
		prop["echo_time"](props.flEchoTime);
		prop["echo_depth"](props.flEchoDepth);
		prop["modulation_time"](props.flModulationTime);
		prop["modulation_depth"](props.flModulationDepth);
		prop["air_absorption_gainhf"](props.flAirAbsorptionGainHF);
		prop["hfreference"](props.flHFReference);
		prop["lfreference"](props.flLFReference);
		prop["room_rolloff_factor"](props.flRoomRolloffFactor);

		prop["decay_hflimit"](props.iDecayHFLimit);

		const std::unordered_map<std::string, float *> vectorKeys = {{"reflections_pan", props.flReflectionsPan.data()}, {"late_reverb_pan", props.flLateReverbPan.data()}};
		for(auto &pair : vectorKeys) {
			std::string val;
			prop[pair.first](val);
			if(!val.empty())
				ustring::string_to_array<float>(val, pair.second, atof, 3);
		}
		return (name != nullptr) ? pragma::get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	return nullptr;
}

std::shared_ptr<al::IEffect> al::create_aux_effect(const std::string &type, udm::LinkedPropertyWrapper &prop) { return create_aux_effect(nullptr, type, prop); }
std::shared_ptr<al::IEffect> al::create_aux_effect(const std::string &name, const std::string &type, udm::LinkedPropertyWrapper &prop) { return create_aux_effect(&name, type, prop); }
