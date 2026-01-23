// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.audio.dsp.base_reverb;

export class DLLNETWORK BaseEnvSoundDspReverb {
  protected:
	float m_kvDensity;
	float m_kvDiffusion;
	float m_kvGain;
	float m_kvGainHF;
	float m_kvDecay;
	float m_kvDecayHF;
	float m_kvReflectionsGain;
	float m_kvReflectionsDelay;
	float m_kvLateGain;
	float m_kvLateDelay;
	float m_kvRoomRolloff;
	float m_kvAirAbsorpGainHF;
	int m_kvDecayLimit;
	BaseEnvSoundDspReverb();
};
