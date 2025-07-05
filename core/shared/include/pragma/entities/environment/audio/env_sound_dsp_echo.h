// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENV_SOUND_DSP_ECHO_H__
#define __ENV_SOUND_DSP_ECHO_H__

#include "pragma/networkdefinitions.h"
#include <string>
class DLLNETWORK BaseEnvSoundDspEcho {
  protected:
	float m_kvDelay = 0.1f;
	float m_kvLRDelay = 0.1f;
	float m_kvDamping = 0.5f;
	float m_kvFeedback = 0.5f;
	float m_kvSpread = -1.f;
	BaseEnvSoundDspEcho();
};

#endif
