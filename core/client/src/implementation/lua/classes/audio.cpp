// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.bindings.audio;

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
#ifdef WINDOWS_CLANG_COMPILER_FIX
		return luabind::object {};
#else
		return nil;
#endif
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
	auto classDefAlEffect = luabind::class_<audio::PEffect>("Effect");
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxEaxReverbProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxChorusProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxDistortionProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxEchoProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxFlangerProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxFrequencyShifterProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxVocalMorpherProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxPitchShifterProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxRingModulatorProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxAutoWahProperties &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxCompressor &props) { effect->SetProperties(props); });
	classDefAlEffect.def("SetProperties", +[](lua::State *, audio::PEffect &effect, const audio::EfxEqualizer &props) { effect->SetProperties(props); });

	auto classDefEaxReverb = luabind::class_<audio::EfxEaxReverbProperties>("ReverbProperties");
	classDefEaxReverb.def(luabind::constructor<>());
	classDefEaxReverb.def_readwrite("density", &audio::EfxEaxReverbProperties::flDensity);
	classDefEaxReverb.def_readwrite("diffusion", &audio::EfxEaxReverbProperties::flDiffusion);
	classDefEaxReverb.def_readwrite("gain", &audio::EfxEaxReverbProperties::flGain);
	classDefEaxReverb.def_readwrite("gainHF", &audio::EfxEaxReverbProperties::flGainHF);
	classDefEaxReverb.def_readwrite("gainLF", &audio::EfxEaxReverbProperties::flGainLF);
	classDefEaxReverb.def_readwrite("decayTime", &audio::EfxEaxReverbProperties::flDecayTime);
	classDefEaxReverb.def_readwrite("decayHFRatio", &audio::EfxEaxReverbProperties::flDecayHFRatio);
	classDefEaxReverb.def_readwrite("decayLFRatio", &audio::EfxEaxReverbProperties::flDecayLFRatio);
	classDefEaxReverb.def_readwrite("reflectionsGain", &audio::EfxEaxReverbProperties::flReflectionsGain);
	classDefEaxReverb.def_readwrite("reflectionsDelay", &audio::EfxEaxReverbProperties::flReflectionsDelay);
	classDefEaxReverb.def_readwrite("reflectionsPan", reinterpret_cast<Vector3 audio::EfxEaxReverbProperties::*>(&audio::EfxEaxReverbProperties::flReflectionsPan));
	classDefEaxReverb.def_readwrite("lateReverbGain", &audio::EfxEaxReverbProperties::flLateReverbGain);
	classDefEaxReverb.def_readwrite("lateReverbDelay", &audio::EfxEaxReverbProperties::flLateReverbDelay);
	classDefEaxReverb.def_readwrite("lateReverbPan", reinterpret_cast<Vector3 audio::EfxEaxReverbProperties::*>(&audio::EfxEaxReverbProperties::flLateReverbPan));
	classDefEaxReverb.def_readwrite("echoTime", &audio::EfxEaxReverbProperties::flEchoTime);
	classDefEaxReverb.def_readwrite("echoDepth", &audio::EfxEaxReverbProperties::flEchoDepth);
	classDefEaxReverb.def_readwrite("modulationTime", &audio::EfxEaxReverbProperties::flModulationTime);
	classDefEaxReverb.def_readwrite("modulationDepth", &audio::EfxEaxReverbProperties::flModulationDepth);
	classDefEaxReverb.def_readwrite("airAbsorptionGainHF", &audio::EfxEaxReverbProperties::flAirAbsorptionGainHF);
	classDefEaxReverb.def_readwrite("hfReference", &audio::EfxEaxReverbProperties::flHFReference);
	classDefEaxReverb.def_readwrite("lfReference", &audio::EfxEaxReverbProperties::flLFReference);
	classDefEaxReverb.def_readwrite("roomRolloffFactor", &audio::EfxEaxReverbProperties::flRoomRolloffFactor);
	classDefEaxReverb.def_readwrite("decayHFLimit", &audio::EfxEaxReverbProperties::iDecayHFLimit);
	classDefAlEffect.scope[classDefEaxReverb];

	auto classDefChorus = luabind::class_<audio::EfxChorusProperties>("ChorusProperties");
	classDefChorus.def(luabind::constructor<>());
	classDefChorus.def_readwrite("waveform", &audio::EfxChorusProperties::iWaveform);
	classDefChorus.def_readwrite("phase", &audio::EfxChorusProperties::iPhase);
	classDefChorus.def_readwrite("rate", &audio::EfxChorusProperties::flRate);
	classDefChorus.def_readwrite("depth", &audio::EfxChorusProperties::flDepth);
	classDefChorus.def_readwrite("feedback", &audio::EfxChorusProperties::flFeedback);
	classDefChorus.def_readwrite("delay", &audio::EfxChorusProperties::flDelay);
	classDefAlEffect.scope[classDefChorus];

	auto classDefDistortionProperties = luabind::class_<audio::EfxDistortionProperties>("DistortionProperties");
	classDefDistortionProperties.def(luabind::constructor<>());
	classDefDistortionProperties.def_readwrite("edge", &audio::EfxDistortionProperties::flEdge);
	classDefDistortionProperties.def_readwrite("gain", &audio::EfxDistortionProperties::flGain);
	classDefDistortionProperties.def_readwrite("lowpassCutoff", &audio::EfxDistortionProperties::flLowpassCutoff);
	classDefDistortionProperties.def_readwrite("eqCenter", &audio::EfxDistortionProperties::flEQCenter);
	classDefDistortionProperties.def_readwrite("eqBandwidth", &audio::EfxDistortionProperties::flEQBandwidth);
	classDefAlEffect.scope[classDefDistortionProperties];

	auto classDefEchoProperties = luabind::class_<audio::EfxEchoProperties>("EchoProperties");
	classDefEchoProperties.def(luabind::constructor<>());
	classDefEchoProperties.def_readwrite("delay", &audio::EfxEchoProperties::flDelay);
	classDefEchoProperties.def_readwrite("lrDelay", &audio::EfxEchoProperties::flLRDelay);
	classDefEchoProperties.def_readwrite("damping", &audio::EfxEchoProperties::flDamping);
	classDefEchoProperties.def_readwrite("feedback", &audio::EfxEchoProperties::flFeedback);
	classDefEchoProperties.def_readwrite("spread", &audio::EfxEchoProperties::flSpread);
	classDefAlEffect.scope[classDefEchoProperties];

	auto classDefFlangerProperties = luabind::class_<audio::EfxFlangerProperties>("FlangerProperties");
	classDefFlangerProperties.def(luabind::constructor<>());
	classDefFlangerProperties.def_readwrite("waveform", &audio::EfxFlangerProperties::iWaveform);
	classDefFlangerProperties.def_readwrite("phase", &audio::EfxFlangerProperties::iPhase);
	classDefFlangerProperties.def_readwrite("rate", &audio::EfxFlangerProperties::flRate);
	classDefFlangerProperties.def_readwrite("depth", &audio::EfxFlangerProperties::flDepth);
	classDefFlangerProperties.def_readwrite("feedback", &audio::EfxFlangerProperties::flFeedback);
	classDefFlangerProperties.def_readwrite("delay", &audio::EfxFlangerProperties::flDelay);
	classDefAlEffect.scope[classDefFlangerProperties];

	auto classDefFrequencyShifterProperties = luabind::class_<audio::EfxFrequencyShifterProperties>("FrequencyShifterProperties");
	classDefFrequencyShifterProperties.def(luabind::constructor<>());
	classDefFrequencyShifterProperties.def_readwrite("frequency", &audio::EfxFrequencyShifterProperties::flFrequency);
	classDefFrequencyShifterProperties.def_readwrite("leftDirection", &audio::EfxFrequencyShifterProperties::iLeftDirection);
	classDefFrequencyShifterProperties.def_readwrite("rightDirection", &audio::EfxFrequencyShifterProperties::iRightDirection);
	classDefAlEffect.scope[classDefFrequencyShifterProperties];

	auto classDefVocalMorpherProperties = luabind::class_<audio::EfxVocalMorpherProperties>("VocalMorpherProperties");
	classDefVocalMorpherProperties.def(luabind::constructor<>());
	classDefVocalMorpherProperties.def_readwrite("phonemeA", &audio::EfxVocalMorpherProperties::iPhonemeA);
	classDefVocalMorpherProperties.def_readwrite("phonemeB", &audio::EfxVocalMorpherProperties::iPhonemeB);
	classDefVocalMorpherProperties.def_readwrite("phonemeACoarseTuning", &audio::EfxVocalMorpherProperties::iPhonemeACoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("phonemeBCoarseTuning", &audio::EfxVocalMorpherProperties::iPhonemeBCoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("waveform", &audio::EfxVocalMorpherProperties::iWaveform);
	classDefVocalMorpherProperties.def_readwrite("rate", &audio::EfxVocalMorpherProperties::flRate);
	classDefAlEffect.scope[classDefVocalMorpherProperties];

	auto classDefPitchShifterProperties = luabind::class_<audio::EfxPitchShifterProperties>("PitchShifterProperties");
	classDefPitchShifterProperties.def(luabind::constructor<>());
	classDefPitchShifterProperties.def_readwrite("coarseTune", &audio::EfxPitchShifterProperties::iCoarseTune);
	classDefPitchShifterProperties.def_readwrite("fineTune", &audio::EfxPitchShifterProperties::iFineTune);
	classDefAlEffect.scope[classDefPitchShifterProperties];

	auto classDefRingModulatorProperties = luabind::class_<audio::EfxRingModulatorProperties>("RingModulatorProperties");
	classDefRingModulatorProperties.def(luabind::constructor<>());
	classDefRingModulatorProperties.def_readwrite("frequency", &audio::EfxRingModulatorProperties::flFrequency);
	classDefRingModulatorProperties.def_readwrite("highpassCutoff", &audio::EfxRingModulatorProperties::flHighpassCutoff);
	classDefRingModulatorProperties.def_readwrite("waveform", &audio::EfxRingModulatorProperties::iWaveform);
	classDefAlEffect.scope[classDefRingModulatorProperties];

	auto classDefAutoWahProperties = luabind::class_<audio::EfxAutoWahProperties>("AutoWahProperties");
	classDefAutoWahProperties.def(luabind::constructor<>());
	classDefAutoWahProperties.def_readwrite("attackTime", &audio::EfxAutoWahProperties::flAttackTime);
	classDefAutoWahProperties.def_readwrite("releaseTime", &audio::EfxAutoWahProperties::flReleaseTime);
	classDefAutoWahProperties.def_readwrite("resonance", &audio::EfxAutoWahProperties::flResonance);
	classDefAutoWahProperties.def_readwrite("peakGain", &audio::EfxAutoWahProperties::flPeakGain);
	classDefAlEffect.scope[classDefAutoWahProperties];

	auto classDefCompressor = luabind::class_<audio::EfxCompressor>("CompressorProperties");
	classDefCompressor.def(luabind::constructor<>());
	classDefCompressor.def_readwrite("onOff", &audio::EfxCompressor::iOnOff);
	classDefAlEffect.scope[classDefCompressor];

	auto classDefEqualizer = luabind::class_<audio::EfxEqualizer>("EqualizerProperties");
	classDefEqualizer.def(luabind::constructor<>());
	classDefEqualizer.def_readwrite("lowGain", &audio::EfxEqualizer::flLowGain);
	classDefEqualizer.def_readwrite("lowCutoff", &audio::EfxEqualizer::flLowCutoff);
	classDefEqualizer.def_readwrite("mid1Gain", &audio::EfxEqualizer::flMid1Gain);
	classDefEqualizer.def_readwrite("mid1Center", &audio::EfxEqualizer::flMid1Center);
	classDefEqualizer.def_readwrite("mid1Width", &audio::EfxEqualizer::flMid1Width);
	classDefEqualizer.def_readwrite("mid2Gain", &audio::EfxEqualizer::flMid2Gain);
	classDefEqualizer.def_readwrite("mid2Center", &audio::EfxEqualizer::flMid2Center);
	classDefEqualizer.def_readwrite("mid2Width", &audio::EfxEqualizer::flMid2Width);
	classDefEqualizer.def_readwrite("highGain", &audio::EfxEqualizer::flHighGain);
	classDefEqualizer.def_readwrite("highCutoff", &audio::EfxEqualizer::flHighCutoff);
	classDefAlEffect.scope[classDefEqualizer];

	auto alSoundClassDef = luabind::class_<audio::ALSound>("Source");
	Lua::ALSound::Client::register_class(alSoundClassDef);

	auto alBufferClassDef = luabind::class_<audio::ISoundBuffer>("Source");
	Lua::ALSound::Client::register_buffer(alBufferClassDef);

	auto soundMod = luabind::module(l, "sound");
	soundMod[classDefAlEffect];
	soundMod[alSoundClassDef];
	soundMod[alBufferClassDef];
}
