// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :audio.aux_util;
import :engine;

static uint32_t s_globalEffectId = std::numeric_limits<uint32_t>::max();
static void debug_audio_aux_effect(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr) {
		Con::CWAR << "Sound engine hasn't been initialized!" << Con::endl;
		return;
	}
	if(argv.empty() == true) {
		if(s_globalEffectId != std::numeric_limits<uint32_t>::max()) {
			soundSys->RemoveGlobalEffect(s_globalEffectId);
			return;
		}
		Con::CWAR << "No effect has been specified!" << Con::endl;
		return;
	}
	auto &dspName = argv.front();
	auto effect = pragma::get_cengine()->GetAuxEffect(dspName);
	if(effect == nullptr) {
		Con::CWAR << "No auxiliary effect found with name '" << dspName << "'!" << Con::endl;
		return;
	}
	s_globalEffectId = soundSys->AddGlobalEffect(*effect); // TODO: Gain
	if(s_globalEffectId == std::numeric_limits<uint32_t>::max())
		Con::CWAR << "Unable to apply auxiliary effect '" << dspName << "'!" << Con::endl;
	else
		Con::COUT << "Auxiliary effect '" << dspName << "' has been applied!" << Con::endl;
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_audio_aux_effect", &debug_audio_aux_effect, pragma::console::ConVarFlags::None, "Applies a global DSP effect. Usage: debug_audio_aux_effect <dspName> <gain>");
}

const std::vector<std::string> &pragma::audio::get_aux_types()
{
	static std::vector<std::string> types = {"reverb", "chorus", "distortion", "echo", "flanger", "frequency_shifter", "vocal_morpher", "pitch_shifter", "ring_modulator", "autowah", "compressor", "equalizer", "eaxreverb"};
	return types;
}

namespace pragma::audio {
	static std::shared_ptr<IEffect> create_aux_effect(const std::string *name, const std::string &type, udm::LinkedPropertyWrapper &prop);
};

std::shared_ptr<pragma::audio::IEffect> pragma::audio::create_aux_effect(const std::string *name, const std::string &type, udm::LinkedPropertyWrapper &prop)
{
	auto *soundSys = get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return nullptr;
	if(type == "reverb") {
		EfxEaxReverbProperties props {};
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
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "chorus") {
		EfxChorusProperties props {};
		prop["rate"](props.flRate);
		prop["depth"](props.flDepth);
		prop["feedback"](props.flFeedback);
		prop["delay"](props.flDelay);

		prop["waveform"](props.iWaveform);
		prop["phase"](props.iPhase);
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "distortion") {
		EfxDistortionProperties props {};
		prop["edge"](props.flEdge);
		prop["gain"](props.flGain);
		prop["lowpass_cutoff"](props.flLowpassCutoff);
		prop["eqcenter"](props.flEQCenter);
		prop["eqbandwidth"](props.flEQBandwidth);
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "echo") {
		EfxEchoProperties props {};
		prop["delay"](props.flDelay);
		prop["lrdelay"](props.flLRDelay);
		prop["damping"](props.flDamping);
		prop["feedback"](props.flFeedback);
		prop["spread"](props.flSpread);
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "flanger") {
		EfxFlangerProperties props {};
		prop["rate"](props.flRate);
		prop["depth"](props.flDepth);
		prop["feedback"](props.flFeedback);
		prop["delay"](props.flDelay);

		prop["waveform"](props.iWaveform);
		prop["phase"](props.iPhase);
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "frequency_shifter") {
		EfxFrequencyShifterProperties props {};
		prop["frequency"](props.flFrequency);

		prop["left_direction"](props.iLeftDirection);
		prop["right_direction"](props.iRightDirection);
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "vocal_morpher") {
		EfxVocalMorpherProperties props {};
		prop["rate"](props.flRate);

		prop["phonemea_coarse_tuning"](props.iPhonemeACoarseTuning);
		prop["phonemeb_coarse_tuning"](props.iPhonemeBCoarseTuning);
		prop["waveform"](props.iWaveform);

		const std::unordered_map<std::string, int32_t> phonemes
		  = {{"a", math::to_integral(VocalMorpherPhoneme::A)}, {"e", math::to_integral(VocalMorpherPhoneme::E)}, {"i", math::to_integral(VocalMorpherPhoneme::I)}, {"o", math::to_integral(VocalMorpherPhoneme::O)}, {"u", math::to_integral(VocalMorpherPhoneme::U)},
		    {"aa", math::to_integral(VocalMorpherPhoneme::AA)}, {"ae", math::to_integral(VocalMorpherPhoneme::AE)}, {"ah", math::to_integral(VocalMorpherPhoneme::AH)}, {"ao", math::to_integral(VocalMorpherPhoneme::AO)}, {"eh", math::to_integral(VocalMorpherPhoneme::EH)},
		    {"er", math::to_integral(VocalMorpherPhoneme::ER)}, {"ih", math::to_integral(VocalMorpherPhoneme::IH)}, {"iy", math::to_integral(VocalMorpherPhoneme::IY)}, {"uh", math::to_integral(VocalMorpherPhoneme::UH)}, {"uw", math::to_integral(VocalMorpherPhoneme::UW)},
		    {"b", math::to_integral(VocalMorpherPhoneme::B)}, {"d", math::to_integral(VocalMorpherPhoneme::D)}, {"f", math::to_integral(VocalMorpherPhoneme::F)}, {"g", math::to_integral(VocalMorpherPhoneme::G)}, {"j", math::to_integral(VocalMorpherPhoneme::J)},
		    {"k", math::to_integral(VocalMorpherPhoneme::K)}, {"l", math::to_integral(VocalMorpherPhoneme::L)}, {"m", math::to_integral(VocalMorpherPhoneme::M)}, {"n", math::to_integral(VocalMorpherPhoneme::N)}, {"p", math::to_integral(VocalMorpherPhoneme::P)},
		    {"r", math::to_integral(VocalMorpherPhoneme::R)}, {"s", math::to_integral(VocalMorpherPhoneme::S)}, {"t", math::to_integral(VocalMorpherPhoneme::T)}, {"v", math::to_integral(VocalMorpherPhoneme::V)}, {"z", math::to_integral(VocalMorpherPhoneme::Z)}};

		const std::unordered_map<std::string, int32_t *> phonemeKeys = {{"phonemea", &props.iPhonemeA}, {"phonemeb", &props.iPhonemeB}};
		for(auto &pair : phonemeKeys) {
			auto &key = pair.first;
			std::string val;
			prop[key](val);
			if(!val.empty()) {
				string::to_lower(val);
				auto itPhoneme = phonemes.find(val);
				if(itPhoneme != phonemes.end())
					*pair.second = itPhoneme->second;
			}
			else
				prop[key](*pair.second);
		}
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "pitch_shifter") {
		EfxPitchShifterProperties props {};
		prop["coarse_tune"](props.iCoarseTune);
		prop["fine_tune"](props.iFineTune);
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "ring_modulator") {
		EfxRingModulatorProperties props {};
		prop["frequency"](props.flFrequency);
		prop["highpass_cutoff"](props.flHighpassCutoff);

		prop["waveform"](props.iWaveform);
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "autowah") {
		EfxAutoWahProperties props {};
		prop["attack_time"](props.flAttackTime);
		prop["release_time"](props.flReleaseTime);
		prop["resonance"](props.flResonance);
		prop["peak_gain"](props.flPeakGain);
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "compressor") {
		EfxCompressor props {};
		prop["onoff"](props.iOnOff);
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "equalizer") {
		EfxEqualizer props {};
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
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	else if(type == "eaxreverb") {
		EfxEaxReverbProperties props {};
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
				pragma::string::string_to_array<float>(val, pair.second, pragma::string::cstring_to_number<float>, 3);
		}
		return (name != nullptr) ? get_cengine()->CreateAuxEffect(*name, props) : soundSys->CreateEffect(props);
	}
	return nullptr;
}

std::shared_ptr<pragma::audio::IEffect> pragma::audio::create_aux_effect(const std::string &type, udm::LinkedPropertyWrapper &prop) { return create_aux_effect(nullptr, type, prop); }
std::shared_ptr<pragma::audio::IEffect> pragma::audio::create_aux_effect(const std::string &name, const std::string &type, udm::LinkedPropertyWrapper &prop) { return create_aux_effect(&name, type, prop); }
