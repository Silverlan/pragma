// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:scripting.lua.bindings.audio;
export import :audio;
export import se_script;

export namespace pragma::scripting::lua_core::bindings {
	void register_audio(lua::State *l);
};

namespace Lua {
	namespace ALSound {
		namespace Client {
			DLLCLIENT void register_class(luabind::class_<pragma::audio::ALSound> &classDef);
			DLLCLIENT void register_buffer(luabind::class_<pragma::audio::ISoundBuffer> &classDef);
		};
	};
};

namespace Lua {
	namespace ALSound {
		namespace Client {
			static void GetWorldPosition(lua::State *l, pragma::audio::ALSound &snd);

			//static void GetChannelConfigName(lua::State *l,::ALSound &snd);
			//static void GetSampleTypeName(lua::State *l,::ALSound &snd);
			static void IsMono(lua::State *l, pragma::audio::ALSound &snd);
			static void IsStereo(lua::State *l, pragma::audio::ALSound &snd);
		};
	};
	namespace ALBuffer {
		static opt<luabind::tableT<void>> GetPhonemeData(lua::State *l, pragma::audio::ISoundBuffer &);
	};
};

void Lua::ALSound::Client::register_buffer(luabind::class_<pragma::audio::ISoundBuffer> &classDef) { classDef.def("GetPhonemeData", &ALBuffer::GetPhonemeData); }

void Lua::ALSound::Client::register_class(luabind::class_<pragma::audio::ALSound> &classDef)
{
	ALSound::register_class(classDef);
	classDef.def("GetWorldPos", &GetWorldPosition);

	//classDef.def("GetChannelConfigName",&Lua::ALSound::Client::GetChannelConfigName);
	//classDef.def("GetSampleTypeName",&Lua::ALSound::Client::GetSampleTypeName);
	classDef.def("IsMono", &IsMono);
	classDef.def("IsStereo", &IsStereo);
	classDef.def("SetPropagationIdentifier", static_cast<void (*)(pragma::audio::ALSound &, const std::string &)>([](pragma::audio::ALSound &sound, const std::string &identifier) { static_cast<pragma::audio::CALSound &>(sound)->SetIdentifier(identifier); }));
	classDef.def("GetPropagationIdentifier", static_cast<std::string (*)(pragma::audio::ALSound &)>([](pragma::audio::ALSound &sound) { return static_cast<pragma::audio::CALSound &>(sound)->GetIdentifier(); }));
	classDef.def("GetBuffer", static_cast<pragma::audio::ISoundBuffer *(*)(pragma::audio::ALSound &)>([](pragma::audio::ALSound &sound) -> pragma::audio::ISoundBuffer * { return static_cast<pragma::audio::CALSound &>(sound)->GetBuffer(); }));
}

void Lua::ALSound::Client::GetWorldPosition(lua::State *l, pragma::audio::ALSound &snd) { Lua::Push<Vector3>(l, static_cast<pragma::audio::CALSound &>(snd)->GetWorldPosition()); }

//void Lua::ALSound::Client::GetChannelConfigName(lua::State *l,pragma::audio::ALSound &snd) {Lua::PushString(l,static_cast<CALSound&>(snd)->GetChannelConfigName());}
//void Lua::ALSound::Client::GetSampleTypeName(lua::State *l,pragma::audio::ALSound &snd) {Lua::PushString(l,static_cast<CALSound&>(snd)->GetSampleTypeName());}
void Lua::ALSound::Client::IsMono(lua::State *l, pragma::audio::ALSound &snd) { PushBool(l, static_cast<pragma::audio::CALSound &>(snd)->IsMono()); }
void Lua::ALSound::Client::IsStereo(lua::State *l, pragma::audio::ALSound &snd) { PushBool(l, static_cast<pragma::audio::CALSound &>(snd)->IsStereo()); }

/////////////////

Lua::opt<luabind::tableT<void>> Lua::ALBuffer::GetPhonemeData(lua::State *l, pragma::audio::ISoundBuffer &buffer)
{
	auto pUserData = buffer.GetUserData();
	if(pUserData == nullptr)
		return nil;
	auto &phonemeData = *static_cast<source_engine::script::SoundPhonemeData *>(pUserData.get());

	auto t = luabind::newtable(l);
	t["plainText"] = phonemeData.plainText;

	auto tWords = t["words"];
	auto idx = 1u;
	for(auto &word : phonemeData.words) {
		auto tWord = tWords[idx++];

		tWord["tStart"] = word.tStart;
		tWord["tEnd"] = word.tEnd;
		tWord["word"] = word.word;

		auto tPhonemes = tWord["phonemes"];
		auto idxPhoneme = 1u;
		for(auto &phoneme : word.phonemes) {
			auto tPhoneme = tPhonemes[idxPhoneme++];
			tPhoneme["phoneme"] = phoneme.phoneme;
			tPhoneme["tStart"] = phoneme.tStart;
			tPhoneme["tEnd"] = phoneme.tEnd;
		}
	}
	return t;
}

void pragma::scripting::lua_core::bindings::register_audio(lua::State *l)
{
	auto classDefAlEffect = luabind::class_<pragma::audio::PEffect>("Effect");
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxEaxReverbProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxChorusProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxDistortionProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxEchoProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxFlangerProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxFrequencyShifterProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxVocalMorpherProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxPitchShifterProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxRingModulatorProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxAutoWahProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxCompressor &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, pragma::audio::PEffect &effect, const pragma::audio::EfxEqualizer &props) { effect->SetProperties(props); });

	auto classDefEaxReverb = luabind::class_<pragma::audio::EfxEaxReverbProperties>("ReverbProperties");
	classDefEaxReverb.def(luabind::constructor<>());
	classDefEaxReverb.def_readwrite("density", &pragma::audio::EfxEaxReverbProperties::flDensity);
	classDefEaxReverb.def_readwrite("diffusion", &pragma::audio::EfxEaxReverbProperties::flDiffusion);
	classDefEaxReverb.def_readwrite("gain", &pragma::audio::EfxEaxReverbProperties::flGain);
	classDefEaxReverb.def_readwrite("gainHF", &pragma::audio::EfxEaxReverbProperties::flGainHF);
	classDefEaxReverb.def_readwrite("gainLF", &pragma::audio::EfxEaxReverbProperties::flGainLF);
	classDefEaxReverb.def_readwrite("decayTime", &pragma::audio::EfxEaxReverbProperties::flDecayTime);
	classDefEaxReverb.def_readwrite("decayHFRatio", &pragma::audio::EfxEaxReverbProperties::flDecayHFRatio);
	classDefEaxReverb.def_readwrite("decayLFRatio", &pragma::audio::EfxEaxReverbProperties::flDecayLFRatio);
	classDefEaxReverb.def_readwrite("reflectionsGain", &pragma::audio::EfxEaxReverbProperties::flReflectionsGain);
	classDefEaxReverb.def_readwrite("reflectionsDelay", &pragma::audio::EfxEaxReverbProperties::flReflectionsDelay);
	classDefEaxReverb.def_readwrite("reflectionsPan", reinterpret_cast<Vector3 pragma::audio::EfxEaxReverbProperties::*>(&pragma::audio::EfxEaxReverbProperties::flReflectionsPan));
	classDefEaxReverb.def_readwrite("lateReverbGain", &pragma::audio::EfxEaxReverbProperties::flLateReverbGain);
	classDefEaxReverb.def_readwrite("lateReverbDelay", &pragma::audio::EfxEaxReverbProperties::flLateReverbDelay);
	classDefEaxReverb.def_readwrite("lateReverbPan", reinterpret_cast<Vector3 pragma::audio::EfxEaxReverbProperties::*>(&pragma::audio::EfxEaxReverbProperties::flLateReverbPan));
	classDefEaxReverb.def_readwrite("echoTime", &pragma::audio::EfxEaxReverbProperties::flEchoTime);
	classDefEaxReverb.def_readwrite("echoDepth", &pragma::audio::EfxEaxReverbProperties::flEchoDepth);
	classDefEaxReverb.def_readwrite("modulationTime", &pragma::audio::EfxEaxReverbProperties::flModulationTime);
	classDefEaxReverb.def_readwrite("modulationDepth", &pragma::audio::EfxEaxReverbProperties::flModulationDepth);
	classDefEaxReverb.def_readwrite("airAbsorptionGainHF", &pragma::audio::EfxEaxReverbProperties::flAirAbsorptionGainHF);
	classDefEaxReverb.def_readwrite("hfReference", &pragma::audio::EfxEaxReverbProperties::flHFReference);
	classDefEaxReverb.def_readwrite("lfReference", &pragma::audio::EfxEaxReverbProperties::flLFReference);
	classDefEaxReverb.def_readwrite("roomRolloffFactor", &pragma::audio::EfxEaxReverbProperties::flRoomRolloffFactor);
	classDefEaxReverb.def_readwrite("decayHFLimit", &pragma::audio::EfxEaxReverbProperties::iDecayHFLimit);
	classDefAlEffect.scope[classDefEaxReverb];

	auto classDefChorus = luabind::class_<pragma::audio::EfxChorusProperties>("ChorusProperties");
	classDefChorus.def(luabind::constructor<>());
	classDefChorus.def_readwrite("waveform", &pragma::audio::EfxChorusProperties::iWaveform);
	classDefChorus.def_readwrite("phase", &pragma::audio::EfxChorusProperties::iPhase);
	classDefChorus.def_readwrite("rate", &pragma::audio::EfxChorusProperties::flRate);
	classDefChorus.def_readwrite("depth", &pragma::audio::EfxChorusProperties::flDepth);
	classDefChorus.def_readwrite("feedback", &pragma::audio::EfxChorusProperties::flFeedback);
	classDefChorus.def_readwrite("delay", &pragma::audio::EfxChorusProperties::flDelay);
	classDefAlEffect.scope[classDefChorus];

	auto classDefDistortionProperties = luabind::class_<pragma::audio::EfxDistortionProperties>("DistortionProperties");
	classDefDistortionProperties.def(luabind::constructor<>());
	classDefDistortionProperties.def_readwrite("edge", &pragma::audio::EfxDistortionProperties::flEdge);
	classDefDistortionProperties.def_readwrite("gain", &pragma::audio::EfxDistortionProperties::flGain);
	classDefDistortionProperties.def_readwrite("lowpassCutoff", &pragma::audio::EfxDistortionProperties::flLowpassCutoff);
	classDefDistortionProperties.def_readwrite("eqCenter", &pragma::audio::EfxDistortionProperties::flEQCenter);
	classDefDistortionProperties.def_readwrite("eqBandwidth", &pragma::audio::EfxDistortionProperties::flEQBandwidth);
	classDefAlEffect.scope[classDefDistortionProperties];

	auto classDefEchoProperties = luabind::class_<pragma::audio::EfxEchoProperties>("EchoProperties");
	classDefEchoProperties.def(luabind::constructor<>());
	classDefEchoProperties.def_readwrite("delay", &pragma::audio::EfxEchoProperties::flDelay);
	classDefEchoProperties.def_readwrite("lrDelay", &pragma::audio::EfxEchoProperties::flLRDelay);
	classDefEchoProperties.def_readwrite("damping", &pragma::audio::EfxEchoProperties::flDamping);
	classDefEchoProperties.def_readwrite("feedback", &pragma::audio::EfxEchoProperties::flFeedback);
	classDefEchoProperties.def_readwrite("spread", &pragma::audio::EfxEchoProperties::flSpread);
	classDefAlEffect.scope[classDefEchoProperties];

	auto classDefFlangerProperties = luabind::class_<pragma::audio::EfxFlangerProperties>("FlangerProperties");
	classDefFlangerProperties.def(luabind::constructor<>());
	classDefFlangerProperties.def_readwrite("waveform", &pragma::audio::EfxFlangerProperties::iWaveform);
	classDefFlangerProperties.def_readwrite("phase", &pragma::audio::EfxFlangerProperties::iPhase);
	classDefFlangerProperties.def_readwrite("rate", &pragma::audio::EfxFlangerProperties::flRate);
	classDefFlangerProperties.def_readwrite("depth", &pragma::audio::EfxFlangerProperties::flDepth);
	classDefFlangerProperties.def_readwrite("feedback", &pragma::audio::EfxFlangerProperties::flFeedback);
	classDefFlangerProperties.def_readwrite("delay", &pragma::audio::EfxFlangerProperties::flDelay);
	classDefAlEffect.scope[classDefFlangerProperties];

	auto classDefFrequencyShifterProperties = luabind::class_<pragma::audio::EfxFrequencyShifterProperties>("FrequencyShifterProperties");
	classDefFrequencyShifterProperties.def(luabind::constructor<>());
	classDefFrequencyShifterProperties.def_readwrite("frequency", &pragma::audio::EfxFrequencyShifterProperties::flFrequency);
	classDefFrequencyShifterProperties.def_readwrite("leftDirection", &pragma::audio::EfxFrequencyShifterProperties::iLeftDirection);
	classDefFrequencyShifterProperties.def_readwrite("rightDirection", &pragma::audio::EfxFrequencyShifterProperties::iRightDirection);
	classDefAlEffect.scope[classDefFrequencyShifterProperties];

	auto classDefVocalMorpherProperties = luabind::class_<pragma::audio::EfxVocalMorpherProperties>("VocalMorpherProperties");
	classDefVocalMorpherProperties.def(luabind::constructor<>());
	classDefVocalMorpherProperties.def_readwrite("phonemeA", &pragma::audio::EfxVocalMorpherProperties::iPhonemeA);
	classDefVocalMorpherProperties.def_readwrite("phonemeB", &pragma::audio::EfxVocalMorpherProperties::iPhonemeB);
	classDefVocalMorpherProperties.def_readwrite("phonemeACoarseTuning", &pragma::audio::EfxVocalMorpherProperties::iPhonemeACoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("phonemeBCoarseTuning", &pragma::audio::EfxVocalMorpherProperties::iPhonemeBCoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("waveform", &pragma::audio::EfxVocalMorpherProperties::iWaveform);
	classDefVocalMorpherProperties.def_readwrite("rate", &pragma::audio::EfxVocalMorpherProperties::flRate);
	classDefAlEffect.scope[classDefVocalMorpherProperties];

	auto classDefPitchShifterProperties = luabind::class_<pragma::audio::EfxPitchShifterProperties>("PitchShifterProperties");
	classDefPitchShifterProperties.def(luabind::constructor<>());
	classDefPitchShifterProperties.def_readwrite("coarseTune", &pragma::audio::EfxPitchShifterProperties::iCoarseTune);
	classDefPitchShifterProperties.def_readwrite("fineTune", &pragma::audio::EfxPitchShifterProperties::iFineTune);
	classDefAlEffect.scope[classDefPitchShifterProperties];

	auto classDefRingModulatorProperties = luabind::class_<pragma::audio::EfxRingModulatorProperties>("RingModulatorProperties");
	classDefRingModulatorProperties.def(luabind::constructor<>());
	classDefRingModulatorProperties.def_readwrite("frequency", &pragma::audio::EfxRingModulatorProperties::flFrequency);
	classDefRingModulatorProperties.def_readwrite("highpassCutoff", &pragma::audio::EfxRingModulatorProperties::flHighpassCutoff);
	classDefRingModulatorProperties.def_readwrite("waveform", &pragma::audio::EfxRingModulatorProperties::iWaveform);
	classDefAlEffect.scope[classDefRingModulatorProperties];

	auto classDefAutoWahProperties = luabind::class_<pragma::audio::EfxAutoWahProperties>("AutoWahProperties");
	classDefAutoWahProperties.def(luabind::constructor<>());
	classDefAutoWahProperties.def_readwrite("attackTime", &pragma::audio::EfxAutoWahProperties::flAttackTime);
	classDefAutoWahProperties.def_readwrite("releaseTime", &pragma::audio::EfxAutoWahProperties::flReleaseTime);
	classDefAutoWahProperties.def_readwrite("resonance", &pragma::audio::EfxAutoWahProperties::flResonance);
	classDefAutoWahProperties.def_readwrite("peakGain", &pragma::audio::EfxAutoWahProperties::flPeakGain);
	classDefAlEffect.scope[classDefAutoWahProperties];

	auto classDefCompressor = luabind::class_<pragma::audio::EfxCompressor>("CompressorProperties");
	classDefCompressor.def(luabind::constructor<>());
	classDefCompressor.def_readwrite("onOff", &pragma::audio::EfxCompressor::iOnOff);
	classDefAlEffect.scope[classDefCompressor];

	auto classDefEqualizer = luabind::class_<pragma::audio::EfxEqualizer>("EqualizerProperties");
	classDefEqualizer.def(luabind::constructor<>());
	classDefEqualizer.def_readwrite("lowGain", &pragma::audio::EfxEqualizer::flLowGain);
	classDefEqualizer.def_readwrite("lowCutoff", &pragma::audio::EfxEqualizer::flLowCutoff);
	classDefEqualizer.def_readwrite("mid1Gain", &pragma::audio::EfxEqualizer::flMid1Gain);
	classDefEqualizer.def_readwrite("mid1Center", &pragma::audio::EfxEqualizer::flMid1Center);
	classDefEqualizer.def_readwrite("mid1Width", &pragma::audio::EfxEqualizer::flMid1Width);
	classDefEqualizer.def_readwrite("mid2Gain", &pragma::audio::EfxEqualizer::flMid2Gain);
	classDefEqualizer.def_readwrite("mid2Center", &pragma::audio::EfxEqualizer::flMid2Center);
	classDefEqualizer.def_readwrite("mid2Width", &pragma::audio::EfxEqualizer::flMid2Width);
	classDefEqualizer.def_readwrite("highGain", &pragma::audio::EfxEqualizer::flHighGain);
	classDefEqualizer.def_readwrite("highCutoff", &pragma::audio::EfxEqualizer::flHighCutoff);
	classDefAlEffect.scope[classDefEqualizer];

	auto alSoundClassDef = luabind::class_<audio::ALSound>("Source");
	Lua::ALSound::Client::register_class(alSoundClassDef);

	auto alBufferClassDef = luabind::class_<pragma::audio::ISoundBuffer>("Source");
	Lua::ALSound::Client::register_buffer(alBufferClassDef);

	auto soundMod = luabind::module(l, "sound");
	soundMod[classDefAlEffect];
	soundMod[alSoundClassDef];
	soundMod[alBufferClassDef];
}
