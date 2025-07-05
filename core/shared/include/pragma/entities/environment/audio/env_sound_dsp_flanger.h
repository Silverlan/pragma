// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENV_SOUND_DSP_FLANGER_H__
#define __ENV_SOUND_DSP_FLANGER_H__

#include "pragma/networkdefinitions.h"
#include <string>
class DLLNETWORK BaseEnvSoundDspFlanger {
  protected:
	int32_t m_kvWaveform = 1;
	int32_t m_kvPhase = 90;
	float m_kvRate = 1.1f;
	float m_kvDepth = 0.1f;
	float m_kvFeedback = 0.25f;
	float m_kvDelay = 0.016f;
	BaseEnvSoundDspFlanger();
};

#endif
