// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"


export module pragma.client:scripting.lua.bindings.audio;
import :audio;
import se_script;

export namespace pragma::scripting::lua::bindings {
	void register_audio(lua_State *l);
};

namespace Lua {
	namespace ALSound {
		namespace Client {
			DLLCLIENT void register_class(luabind::class_<::ALSound> &classDef);
			DLLCLIENT void register_buffer(luabind::class_<al::ISoundBuffer> &classDef);
		};
	};
};

namespace Lua {
	namespace ALSound {
		namespace Client {
			static void GetWorldPosition(lua_State *l, ::ALSound &snd);

			//static void GetChannelConfigName(lua_State *l,::ALSound &snd);
			//static void GetSampleTypeName(lua_State *l,::ALSound &snd);
			static void IsMono(lua_State *l, ::ALSound &snd);
			static void IsStereo(lua_State *l, ::ALSound &snd);
		};
	};
	namespace ALBuffer {
		static Lua::opt<luabind::tableT<void>> GetPhonemeData(lua_State *l, al::ISoundBuffer &);
	};
};

void Lua::ALSound::Client::register_buffer(luabind::class_<al::ISoundBuffer> &classDef) { classDef.def("GetPhonemeData", &Lua::ALBuffer::GetPhonemeData); }

void Lua::ALSound::Client::register_class(luabind::class_<::ALSound> &classDef)
{
	ALSound::register_class(classDef);
	classDef.def("GetWorldPos", &Lua::ALSound::Client::GetWorldPosition);

	//classDef.def("GetChannelConfigName",&Lua::ALSound::Client::GetChannelConfigName);
	//classDef.def("GetSampleTypeName",&Lua::ALSound::Client::GetSampleTypeName);
	classDef.def("IsMono", &Lua::ALSound::Client::IsMono);
	classDef.def("IsStereo", &Lua::ALSound::Client::IsStereo);
	classDef.def("SetPropagationIdentifier", static_cast<void (*)(::ALSound &, const std::string &)>([](::ALSound &sound, const std::string &identifier) { static_cast<CALSound &>(sound)->SetIdentifier(identifier); }));
	classDef.def("GetPropagationIdentifier", static_cast<std::string (*)(::ALSound &)>([](::ALSound &sound) { return static_cast<CALSound &>(sound)->GetIdentifier(); }));
	classDef.def("GetBuffer", static_cast<al::ISoundBuffer *(*)(::ALSound &)>([](::ALSound &sound) -> al::ISoundBuffer * { return static_cast<CALSound &>(sound)->GetBuffer(); }));
}

void Lua::ALSound::Client::GetWorldPosition(lua_State *l, ::ALSound &snd) { Lua::Push<Vector3>(l, static_cast<CALSound &>(snd)->GetWorldPosition()); }

//void Lua::ALSound::Client::GetChannelConfigName(lua_State *l,::ALSound &snd) {Lua::PushString(l,static_cast<CALSound&>(snd)->GetChannelConfigName());}
//void Lua::ALSound::Client::GetSampleTypeName(lua_State *l,::ALSound &snd) {Lua::PushString(l,static_cast<CALSound&>(snd)->GetSampleTypeName());}
void Lua::ALSound::Client::IsMono(lua_State *l, ::ALSound &snd) { Lua::PushBool(l, static_cast<CALSound &>(snd)->IsMono()); }
void Lua::ALSound::Client::IsStereo(lua_State *l, ::ALSound &snd) { Lua::PushBool(l, static_cast<CALSound &>(snd)->IsStereo()); }

/////////////////

Lua::opt<luabind::tableT<void>> Lua::ALBuffer::GetPhonemeData(lua_State *l, al::ISoundBuffer &buffer)
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

void pragma::scripting::lua::bindings::register_audio(lua_State *l) {
	auto classDefAlEffect = luabind::class_<al::PEffect>("Effect");
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxEaxReverbProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxChorusProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxDistortionProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxEchoProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxFlangerProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxFrequencyShifterProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxVocalMorpherProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxPitchShifterProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxRingModulatorProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxAutoWahProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxCompressor &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua_State *, al::PEffect &effect, const al::EfxEqualizer &props) { effect->SetProperties(props); });

	auto classDefEaxReverb = luabind::class_<al::EfxEaxReverbProperties>("ReverbProperties");
	classDefEaxReverb.def(luabind::constructor<>());
	classDefEaxReverb.def_readwrite("density", &al::EfxEaxReverbProperties::flDensity);
	classDefEaxReverb.def_readwrite("diffusion", &al::EfxEaxReverbProperties::flDiffusion);
	classDefEaxReverb.def_readwrite("gain", &al::EfxEaxReverbProperties::flGain);
	classDefEaxReverb.def_readwrite("gainHF", &al::EfxEaxReverbProperties::flGainHF);
	classDefEaxReverb.def_readwrite("gainLF", &al::EfxEaxReverbProperties::flGainLF);
	classDefEaxReverb.def_readwrite("decayTime", &al::EfxEaxReverbProperties::flDecayTime);
	classDefEaxReverb.def_readwrite("decayHFRatio", &al::EfxEaxReverbProperties::flDecayHFRatio);
	classDefEaxReverb.def_readwrite("decayLFRatio", &al::EfxEaxReverbProperties::flDecayLFRatio);
	classDefEaxReverb.def_readwrite("reflectionsGain", &al::EfxEaxReverbProperties::flReflectionsGain);
	classDefEaxReverb.def_readwrite("reflectionsDelay", &al::EfxEaxReverbProperties::flReflectionsDelay);
	classDefEaxReverb.def_readwrite("reflectionsPan", reinterpret_cast<Vector3 al::EfxEaxReverbProperties::*>(&al::EfxEaxReverbProperties::flReflectionsPan));
	classDefEaxReverb.def_readwrite("lateReverbGain", &al::EfxEaxReverbProperties::flLateReverbGain);
	classDefEaxReverb.def_readwrite("lateReverbDelay", &al::EfxEaxReverbProperties::flLateReverbDelay);
	classDefEaxReverb.def_readwrite("lateReverbPan", reinterpret_cast<Vector3 al::EfxEaxReverbProperties::*>(&al::EfxEaxReverbProperties::flLateReverbPan));
	classDefEaxReverb.def_readwrite("echoTime", &al::EfxEaxReverbProperties::flEchoTime);
	classDefEaxReverb.def_readwrite("echoDepth", &al::EfxEaxReverbProperties::flEchoDepth);
	classDefEaxReverb.def_readwrite("modulationTime", &al::EfxEaxReverbProperties::flModulationTime);
	classDefEaxReverb.def_readwrite("modulationDepth", &al::EfxEaxReverbProperties::flModulationDepth);
	classDefEaxReverb.def_readwrite("airAbsorptionGainHF", &al::EfxEaxReverbProperties::flAirAbsorptionGainHF);
	classDefEaxReverb.def_readwrite("hfReference", &al::EfxEaxReverbProperties::flHFReference);
	classDefEaxReverb.def_readwrite("lfReference", &al::EfxEaxReverbProperties::flLFReference);
	classDefEaxReverb.def_readwrite("roomRolloffFactor", &al::EfxEaxReverbProperties::flRoomRolloffFactor);
	classDefEaxReverb.def_readwrite("decayHFLimit", &al::EfxEaxReverbProperties::iDecayHFLimit);
	classDefAlEffect.scope[classDefEaxReverb];

	auto classDefChorus = luabind::class_<al::EfxChorusProperties>("ChorusProperties");
	classDefChorus.def(luabind::constructor<>());
	classDefChorus.def_readwrite("waveform", &al::EfxChorusProperties::iWaveform);
	classDefChorus.def_readwrite("phase", &al::EfxChorusProperties::iPhase);
	classDefChorus.def_readwrite("rate", &al::EfxChorusProperties::flRate);
	classDefChorus.def_readwrite("depth", &al::EfxChorusProperties::flDepth);
	classDefChorus.def_readwrite("feedback", &al::EfxChorusProperties::flFeedback);
	classDefChorus.def_readwrite("delay", &al::EfxChorusProperties::flDelay);
	classDefAlEffect.scope[classDefChorus];

	auto classDefDistortionProperties = luabind::class_<al::EfxDistortionProperties>("DistortionProperties");
	classDefDistortionProperties.def(luabind::constructor<>());
	classDefDistortionProperties.def_readwrite("edge", &al::EfxDistortionProperties::flEdge);
	classDefDistortionProperties.def_readwrite("gain", &al::EfxDistortionProperties::flGain);
	classDefDistortionProperties.def_readwrite("lowpassCutoff", &al::EfxDistortionProperties::flLowpassCutoff);
	classDefDistortionProperties.def_readwrite("eqCenter", &al::EfxDistortionProperties::flEQCenter);
	classDefDistortionProperties.def_readwrite("eqBandwidth", &al::EfxDistortionProperties::flEQBandwidth);
	classDefAlEffect.scope[classDefDistortionProperties];

	auto classDefEchoProperties = luabind::class_<al::EfxEchoProperties>("EchoProperties");
	classDefEchoProperties.def(luabind::constructor<>());
	classDefEchoProperties.def_readwrite("delay", &al::EfxEchoProperties::flDelay);
	classDefEchoProperties.def_readwrite("lrDelay", &al::EfxEchoProperties::flLRDelay);
	classDefEchoProperties.def_readwrite("damping", &al::EfxEchoProperties::flDamping);
	classDefEchoProperties.def_readwrite("feedback", &al::EfxEchoProperties::flFeedback);
	classDefEchoProperties.def_readwrite("spread", &al::EfxEchoProperties::flSpread);
	classDefAlEffect.scope[classDefEchoProperties];

	auto classDefFlangerProperties = luabind::class_<al::EfxFlangerProperties>("FlangerProperties");
	classDefFlangerProperties.def(luabind::constructor<>());
	classDefFlangerProperties.def_readwrite("waveform", &al::EfxFlangerProperties::iWaveform);
	classDefFlangerProperties.def_readwrite("phase", &al::EfxFlangerProperties::iPhase);
	classDefFlangerProperties.def_readwrite("rate", &al::EfxFlangerProperties::flRate);
	classDefFlangerProperties.def_readwrite("depth", &al::EfxFlangerProperties::flDepth);
	classDefFlangerProperties.def_readwrite("feedback", &al::EfxFlangerProperties::flFeedback);
	classDefFlangerProperties.def_readwrite("delay", &al::EfxFlangerProperties::flDelay);
	classDefAlEffect.scope[classDefFlangerProperties];

	auto classDefFrequencyShifterProperties = luabind::class_<al::EfxFrequencyShifterProperties>("FrequencyShifterProperties");
	classDefFrequencyShifterProperties.def(luabind::constructor<>());
	classDefFrequencyShifterProperties.def_readwrite("frequency", &al::EfxFrequencyShifterProperties::flFrequency);
	classDefFrequencyShifterProperties.def_readwrite("leftDirection", &al::EfxFrequencyShifterProperties::iLeftDirection);
	classDefFrequencyShifterProperties.def_readwrite("rightDirection", &al::EfxFrequencyShifterProperties::iRightDirection);
	classDefAlEffect.scope[classDefFrequencyShifterProperties];

	auto classDefVocalMorpherProperties = luabind::class_<al::EfxVocalMorpherProperties>("VocalMorpherProperties");
	classDefVocalMorpherProperties.def(luabind::constructor<>());
	classDefVocalMorpherProperties.def_readwrite("phonemeA", &al::EfxVocalMorpherProperties::iPhonemeA);
	classDefVocalMorpherProperties.def_readwrite("phonemeB", &al::EfxVocalMorpherProperties::iPhonemeB);
	classDefVocalMorpherProperties.def_readwrite("phonemeACoarseTuning", &al::EfxVocalMorpherProperties::iPhonemeACoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("phonemeBCoarseTuning", &al::EfxVocalMorpherProperties::iPhonemeBCoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("waveform", &al::EfxVocalMorpherProperties::iWaveform);
	classDefVocalMorpherProperties.def_readwrite("rate", &al::EfxVocalMorpherProperties::flRate);
	classDefAlEffect.scope[classDefVocalMorpherProperties];

	auto classDefPitchShifterProperties = luabind::class_<al::EfxPitchShifterProperties>("PitchShifterProperties");
	classDefPitchShifterProperties.def(luabind::constructor<>());
	classDefPitchShifterProperties.def_readwrite("coarseTune", &al::EfxPitchShifterProperties::iCoarseTune);
	classDefPitchShifterProperties.def_readwrite("fineTune", &al::EfxPitchShifterProperties::iFineTune);
	classDefAlEffect.scope[classDefPitchShifterProperties];

	auto classDefRingModulatorProperties = luabind::class_<al::EfxRingModulatorProperties>("RingModulatorProperties");
	classDefRingModulatorProperties.def(luabind::constructor<>());
	classDefRingModulatorProperties.def_readwrite("frequency", &al::EfxRingModulatorProperties::flFrequency);
	classDefRingModulatorProperties.def_readwrite("highpassCutoff", &al::EfxRingModulatorProperties::flHighpassCutoff);
	classDefRingModulatorProperties.def_readwrite("waveform", &al::EfxRingModulatorProperties::iWaveform);
	classDefAlEffect.scope[classDefRingModulatorProperties];

	auto classDefAutoWahProperties = luabind::class_<al::EfxAutoWahProperties>("AutoWahProperties");
	classDefAutoWahProperties.def(luabind::constructor<>());
	classDefAutoWahProperties.def_readwrite("attackTime", &al::EfxAutoWahProperties::flAttackTime);
	classDefAutoWahProperties.def_readwrite("releaseTime", &al::EfxAutoWahProperties::flReleaseTime);
	classDefAutoWahProperties.def_readwrite("resonance", &al::EfxAutoWahProperties::flResonance);
	classDefAutoWahProperties.def_readwrite("peakGain", &al::EfxAutoWahProperties::flPeakGain);
	classDefAlEffect.scope[classDefAutoWahProperties];

	auto classDefCompressor = luabind::class_<al::EfxCompressor>("CompressorProperties");
	classDefCompressor.def(luabind::constructor<>());
	classDefCompressor.def_readwrite("onOff", &al::EfxCompressor::iOnOff);
	classDefAlEffect.scope[classDefCompressor];

	auto classDefEqualizer = luabind::class_<al::EfxEqualizer>("EqualizerProperties");
	classDefEqualizer.def(luabind::constructor<>());
	classDefEqualizer.def_readwrite("lowGain", &al::EfxEqualizer::flLowGain);
	classDefEqualizer.def_readwrite("lowCutoff", &al::EfxEqualizer::flLowCutoff);
	classDefEqualizer.def_readwrite("mid1Gain", &al::EfxEqualizer::flMid1Gain);
	classDefEqualizer.def_readwrite("mid1Center", &al::EfxEqualizer::flMid1Center);
	classDefEqualizer.def_readwrite("mid1Width", &al::EfxEqualizer::flMid1Width);
	classDefEqualizer.def_readwrite("mid2Gain", &al::EfxEqualizer::flMid2Gain);
	classDefEqualizer.def_readwrite("mid2Center", &al::EfxEqualizer::flMid2Center);
	classDefEqualizer.def_readwrite("mid2Width", &al::EfxEqualizer::flMid2Width);
	classDefEqualizer.def_readwrite("highGain", &al::EfxEqualizer::flHighGain);
	classDefEqualizer.def_readwrite("highCutoff", &al::EfxEqualizer::flHighCutoff);
	classDefAlEffect.scope[classDefEqualizer];

	auto alSoundClassDef = luabind::class_<ALSound>("Source");
	Lua::ALSound::Client::register_class(alSoundClassDef);

	auto alBufferClassDef = luabind::class_<al::ISoundBuffer>("Source");
	Lua::ALSound::Client::register_buffer(alBufferClassDef);

	auto soundMod = luabind::module(l, "sound");
	soundMod[classDefAlEffect];
	soundMod[alSoundClassDef];
	soundMod[alBufferClassDef];
}
