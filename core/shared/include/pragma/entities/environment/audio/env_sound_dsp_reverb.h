/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_SOUND_DSP_REVERB_H__
#define __ENV_SOUND_DSP_REVERB_H__

#include "pragma/networkdefinitions.h"
#include <string>
#include <mathutil/glmutil.h>
class DLLNETWORK BaseEnvSoundDspReverb
{
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

#endif