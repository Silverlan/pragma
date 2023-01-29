/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_SOUND_DSP_EQUALIZER_H__
#define __ENV_SOUND_DSP_EQUALIZER_H__

#include "pragma/networkdefinitions.h"
#include <string>
class DLLNETWORK BaseEnvSoundDspEqualizer {
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

#endif
