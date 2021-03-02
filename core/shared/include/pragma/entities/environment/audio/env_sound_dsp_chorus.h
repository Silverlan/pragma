/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_SOUND_DSP_CHORUS_H__
#define __ENV_SOUND_DSP_CHORUS_H__

#include "pragma/networkdefinitions.h"
#include <string>
#include <mathutil/glmutil.h>
class DLLNETWORK BaseEnvSoundDspChorus
{
protected:
	int32_t m_kvWaveform = 1;
	int32_t m_kvPhase = 90;
	float m_kvRate = 1.1f;
	float m_kvDepth = 0.1f;
	float m_kvFeedback = 0.25f;
	float m_kvDelay = 0.016f;
	BaseEnvSoundDspChorus();
};

#endif