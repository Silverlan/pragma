// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENV_SOUND_DSP_DISTORTION_H__
#define __ENV_SOUND_DSP_DISTORTION_H__

#include "pragma/networkdefinitions.h"
#include <string>
class DLLNETWORK BaseEnvSoundDspDistortion {
  protected:
	float m_kvEdge = 0.2f;
	float m_kvGain = 0.05f;
	float m_kvLowpassCutoff = 8'000.f;
	float m_kvEqCenter = 3'600.f;
	float m_kvEqBandwidth = 3'600.f;
	BaseEnvSoundDspDistortion();
};

#endif
