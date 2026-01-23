// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.audio.dsp.base_flanger;

export import std.compat;

export class DLLNETWORK BaseEnvSoundDspFlanger {
  protected:
	int32_t m_kvWaveform = 1;
	int32_t m_kvPhase = 90;
	float m_kvRate = 1.1f;
	float m_kvDepth = 0.1f;
	float m_kvFeedback = 0.25f;
	float m_kvDelay = 0.016f;
	BaseEnvSoundDspFlanger();
};
