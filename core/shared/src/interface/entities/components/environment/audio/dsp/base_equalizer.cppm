// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.audio.dsp.base_equalizer;

export class DLLNETWORK BaseEnvSoundDspEqualizer {
  protected:
	float m_lowGain = 1.f;
	float m_lowCutoff = 200.f;
	float m_mid1Gain = 1.f;
	float m_mid1Center = 500.f;
	float m_mid1Width = 1.f;
	float m_mid2Gain = 1.f;
	float m_mid2Center = 3'000.f;
	float m_mid2Width = 1.f;
	float m_highGain = 1.f;
	float m_highCutoff = 6'000.f;
	BaseEnvSoundDspEqualizer();
};
