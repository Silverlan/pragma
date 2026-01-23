// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.audio.dsp.base_eax_reverb;

export import pragma.math;

export class DLLNETWORK BaseEnvSoundDspEAXReverb {
  protected:
	float m_kvDensity = 1.f;
	float m_kvDiffusion = 1.f;
	float m_kvGain = 0.32f;
	float m_kvGainHF = 0.89f;
	float m_kvGainLF = 1.f;
	float m_kvDecay = 1.49f;
	float m_kvDecayHF = 0.83f;
	float m_kvDecayLF = 1.f;
	int32_t m_kvDecayHFLimit = 1;
	float m_kvReflectionsGain = 0.05f;
	float m_kvReflectionsDelay = 0.007f;
	Vector3 m_kvReflectionsPan = {0.f, 0.f, 0.f};
	float m_kvLateGain = 1.26f;
	float m_kvLateDelay = 0.011f;
	Vector3 m_kvLatePan = {0.f, 0.f, 0.f};
	float m_kvEchoTime = 0.25f;
	float m_kvEchoDepth = 0.f;
	float m_kvModTime = 0.25f;
	float m_kvModDepth = 0.f;
	float m_kvRefHF = 5000.f;
	float m_kvRefLF = 250.f;
	float m_kvRoomRolloff = 0.f;
	float m_kvAirAbsorpGainHF = 0.994f;
	BaseEnvSoundDspEAXReverb();
};
