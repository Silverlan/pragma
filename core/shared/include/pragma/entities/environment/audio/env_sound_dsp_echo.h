/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
