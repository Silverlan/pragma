#include "stdafx_client.h"
#include "pragma/audio/c_sound_efx.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include <pragma/c_engine.h>
#include <alsound_effect.hpp>
#include <alsoundsystem.hpp>
#include <datasystem.h>

extern DLLCENGINE CEngine *c_engine;

static uint32_t s_globalEffectId = std::numeric_limits<uint32_t>::max();
void Console::commands::debug_audio_aux_effect(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
	{
		Con::cwar<<"WARNING: Sound engine hasn't been initialized!"<<Con::endl;
		return;
	}
	if(argv.empty() == true)
	{
		if(s_globalEffectId != std::numeric_limits<uint32_t>::max())
		{
			soundSys->RemoveGlobalEffect(s_globalEffectId);
			return;
		}
		Con::cwar<<"WARNING: No effect has been specified!"<<Con::endl;
		return;
	}
	auto &dspName = argv.front();
	auto effect = c_engine->GetAuxEffect(dspName);
	if(effect == nullptr)
	{
		Con::cwar<<"WARNING: No auxiliary effect found with name '"<<dspName<<"'!"<<Con::endl;
		return;
	}
	s_globalEffectId = soundSys->AddGlobalEffect(*effect); // TODO: Gain
	if(s_globalEffectId == std::numeric_limits<uint32_t>::max())
		Con::cwar<<"WARNING: Unable to apply auxiliary effect '"<<dspName<<"'!"<<Con::endl;
	else
		Con::cout<<"Auxiliary effect '"<<dspName<<"' has been applied!"<<Con::endl;
}

const std::vector<std::string> &al::get_aux_types()
{
	static std::vector<std::string> types = {
		"reverb",
		"chorus",
		"distortion",
		"echo",
		"flanger",
		"frequency_shifter",
		"vocal_morpher",
		"pitch_shifter",
		"ring_modulator",
		"autowah",
		"compressor",
		"equalizer",
		"eaxreverb"
	};
	return types;
}

namespace al
{
	static std::shared_ptr<al::Effect> create_aux_effect(const std::string *name,const std::string &type,ds::Block &block);
};

std::shared_ptr<al::Effect> al::create_aux_effect(const std::string *name,const std::string &type,ds::Block &block)
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return nullptr;
	if(type == "reverb")
	{
		al::EfxEaxReverbProperties props {};
		block.GetFloat("density",&props.flDensity);
		block.GetFloat("diffusion",&props.flDiffusion);
		block.GetFloat("gain",&props.flGain);
		block.GetFloat("gainhf",&props.flGainHF);
		block.GetFloat("gainlf",&props.flGainLF);
		block.GetFloat("decay_time",&props.flDecayTime);
		block.GetFloat("decay_hfratio",&props.flDecayHFRatio);
		block.GetFloat("reflections_gain",&props.flReflectionsGain);
		block.GetFloat("reflections_delay",&props.flReflectionsDelay);
		block.GetFloat("late_reverb_gain",&props.flLateReverbGain);
		block.GetFloat("late_reverb_delay",&props.flLateReverbDelay);
		block.GetFloat("air_absorption_gainhf",&props.flAirAbsorptionGainHF);
		block.GetFloat("room_rolloff_factor",&props.flRoomRolloffFactor);
		block.GetInt("decay_hflimit",&props.iDecayHFLimit);
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "chorus")
	{
		al::EfxChorusProperties props {};
		block.GetFloat("rate",&props.flRate);
		block.GetFloat("depth",&props.flDepth);
		block.GetFloat("feedback",&props.flFeedback);
		block.GetFloat("delay",&props.flDelay);

		block.GetInt("waveform",&props.iWaveform);
		block.GetInt("phase",&props.iPhase);
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "distortion")
	{
		al::EfxDistortionProperties props {};
		block.GetFloat("edge",&props.flEdge);
		block.GetFloat("gain",&props.flGain);
		block.GetFloat("lowpass_cutoff",&props.flLowpassCutoff);
		block.GetFloat("eqcenter",&props.flEQCenter);
		block.GetFloat("eqbandwidth",&props.flEQBandwidth);
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "echo")
	{
		al::EfxEchoProperties props {};
		block.GetFloat("delay",&props.flDelay);
		block.GetFloat("lrdelay",&props.flLRDelay);
		block.GetFloat("damping",&props.flDamping);
		block.GetFloat("feedback",&props.flFeedback);
		block.GetFloat("spread",&props.flSpread);
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "flanger")
	{
		al::EfxFlangerProperties props {};
		block.GetFloat("rate",&props.flRate);
		block.GetFloat("depth",&props.flDepth);
		block.GetFloat("feedback",&props.flFeedback);
		block.GetFloat("delay",&props.flDelay);

		block.GetInt("waveform",&props.iWaveform);
		block.GetInt("phase",&props.iPhase);
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "frequency_shifter")
	{
		al::EfxFrequencyShifterProperties props {};
		block.GetFloat("frequency",&props.flFrequency);

		block.GetInt("left_direction",&props.iLeftDirection);
		block.GetInt("right_direction",&props.iRightDirection);
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "vocal_morpher")
	{
		al::EfxVocalMorpherProperties props {};
		block.GetFloat("rate",&props.flRate);

		block.GetInt("phonemea_coarse_tuning",&props.iPhonemeACoarseTuning);
		block.GetInt("phonemeb_coarse_tuning",&props.iPhonemeBCoarseTuning);
		block.GetInt("waveform",&props.iWaveform);

		const std::unordered_map<std::string,int32_t> phonemes = {
			{"a",umath::to_integral(al::VocalMorpherPhoneme::A)},
			{"e",umath::to_integral(al::VocalMorpherPhoneme::E)},
			{"i",umath::to_integral(al::VocalMorpherPhoneme::I)},
			{"o",umath::to_integral(al::VocalMorpherPhoneme::O)},
			{"u",umath::to_integral(al::VocalMorpherPhoneme::U)},
			{"aa",umath::to_integral(al::VocalMorpherPhoneme::AA)},
			{"ae",umath::to_integral(al::VocalMorpherPhoneme::AE)},
			{"ah",umath::to_integral(al::VocalMorpherPhoneme::AH)},
			{"ao",umath::to_integral(al::VocalMorpherPhoneme::AO)},
			{"eh",umath::to_integral(al::VocalMorpherPhoneme::EH)},
			{"er",umath::to_integral(al::VocalMorpherPhoneme::ER)},
			{"ih",umath::to_integral(al::VocalMorpherPhoneme::IH)},
			{"iy",umath::to_integral(al::VocalMorpherPhoneme::IY)},
			{"uh",umath::to_integral(al::VocalMorpherPhoneme::UH)},
			{"uw",umath::to_integral(al::VocalMorpherPhoneme::UW)},
			{"b",umath::to_integral(al::VocalMorpherPhoneme::B)},
			{"d",umath::to_integral(al::VocalMorpherPhoneme::D)},
			{"f",umath::to_integral(al::VocalMorpherPhoneme::F)},
			{"g",umath::to_integral(al::VocalMorpherPhoneme::G)},
			{"j",umath::to_integral(al::VocalMorpherPhoneme::J)},
			{"k",umath::to_integral(al::VocalMorpherPhoneme::K)},
			{"l",umath::to_integral(al::VocalMorpherPhoneme::L)},
			{"m",umath::to_integral(al::VocalMorpherPhoneme::M)},
			{"n",umath::to_integral(al::VocalMorpherPhoneme::N)},
			{"p",umath::to_integral(al::VocalMorpherPhoneme::P)},
			{"r",umath::to_integral(al::VocalMorpherPhoneme::R)},
			{"s",umath::to_integral(al::VocalMorpherPhoneme::S)},
			{"t",umath::to_integral(al::VocalMorpherPhoneme::T)},
			{"v",umath::to_integral(al::VocalMorpherPhoneme::V)},
			{"z",umath::to_integral(al::VocalMorpherPhoneme::Z)}
		};
		
		const std::unordered_map<std::string,int32_t*> phonemeKeys = {
			{"phonemea",&props.iPhonemeA},
			{"phonemeb",&props.iPhonemeB}
		};
		for(auto &pair : phonemeKeys)
		{
			auto &key = pair.first;
			std::string val;
			if(block.GetString(key,&val) == true)
			{
				ustring::to_lower(val);
				auto itPhoneme = phonemes.find(val);
				if(itPhoneme != phonemes.end())
					*pair.second = itPhoneme->second;
			}
			else
				block.GetInt(key,pair.second);
		}
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "pitch_shifter")
	{
		al::EfxPitchShifterProperties props {};
		block.GetInt("coarse_tune",&props.iCoarseTune);
		block.GetInt("fine_tune",&props.iFineTune);
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "ring_modulator")
	{
		al::EfxRingModulatorProperties props {};
		block.GetFloat("frequency",&props.flFrequency);
		block.GetFloat("highpass_cutoff",&props.flHighpassCutoff);

		block.GetInt("waveform",&props.iWaveform);
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "autowah")
	{
		al::EfxAutoWahProperties props {};
		block.GetFloat("attack_time",&props.flAttackTime);
		block.GetFloat("release_time",&props.flReleaseTime);
		block.GetFloat("resonance",&props.flResonance);
		block.GetFloat("peak_gain",&props.flPeakGain);
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "compressor")
	{
		al::EfxCompressor props {};
		block.GetInt("onoff",&props.iOnOff);
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "equalizer")
	{
		al::EfxEqualizer props {};
		block.GetFloat("low_gain",&props.flLowGain);
		block.GetFloat("low_cutoff",&props.flLowCutoff);
		block.GetFloat("mid1_gain",&props.flMid1Gain);
		block.GetFloat("mid1_center",&props.flMid1Center);
		block.GetFloat("mid1_width",&props.flMid1Width);
		block.GetFloat("mid2_gain",&props.flMid2Gain);
		block.GetFloat("mid2_center",&props.flMid2Center);
		block.GetFloat("mid2_width",&props.flMid2Width);
		block.GetFloat("high_gain",&props.flHighGain);
		block.GetFloat("high_cutoff",&props.flHighCutoff);
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	else if(type == "eaxreverb")
	{
		al::EfxEaxReverbProperties props {};
		block.GetFloat("density",&props.flDensity);
		block.GetFloat("diffusion",&props.flDiffusion);
		block.GetFloat("gain",&props.flGain);
		block.GetFloat("gainhf",&props.flGainHF);
		block.GetFloat("gainlf",&props.flGainLF);
		block.GetFloat("decay_time",&props.flDecayTime);
		block.GetFloat("decay_hfratio",&props.flDecayHFRatio);
		block.GetFloat("decay_lfratio",&props.flDecayLFRatio);
		block.GetFloat("reflections_gain",&props.flReflectionsGain);
		block.GetFloat("reflections_delay",&props.flReflectionsDelay);
		block.GetFloat("late_reverb_gain",&props.flLateReverbGain);
		block.GetFloat("late_reverb_delay",&props.flLateReverbDelay);
		block.GetFloat("echo_time",&props.flEchoTime);
		block.GetFloat("echo_depth",&props.flEchoDepth);
		block.GetFloat("modulation_time",&props.flModulationTime);
		block.GetFloat("modulation_depth",&props.flModulationDepth);
		block.GetFloat("air_absorption_gainhf",&props.flAirAbsorptionGainHF);
		block.GetFloat("hfreference",&props.flHFReference);
		block.GetFloat("lfreference",&props.flLFReference);
		block.GetFloat("room_rolloff_factor",&props.flRoomRolloffFactor);

		block.GetInt("decay_hflimit",&props.iDecayHFLimit);

		const std::unordered_map<std::string,float*> vectorKeys = {
			{"reflections_pan",props.flReflectionsPan.data()},
			{"late_reverb_pan",props.flLateReverbPan.data()}
		};
		for(auto &pair : vectorKeys)
		{
			std::string val;
			if(block.GetString(pair.first,&val) == true)
				ustring::string_to_array<float>(val,pair.second,atof,3);
		}
		return (name != nullptr) ? c_engine->CreateAuxEffect(*name,props) : soundSys->CreateEffect(props);
	}
	return nullptr;
}

std::shared_ptr<al::Effect> al::create_aux_effect(const std::string &type,ds::Block &block) {return create_aux_effect(nullptr,type,block);}
std::shared_ptr<al::Effect> al::create_aux_effect(const std::string &name,const std::string &type,ds::Block &block) {return create_aux_effect(&name,type,block);}
