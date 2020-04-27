/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __ENV_SOUND_DSP_DISTORTION_H__
#define __ENV_SOUND_DSP_DISTORTION_H__

#include "pragma/networkdefinitions.h"
#include <string>
class DLLNETWORK BaseEnvSoundDspDistortion
{
protected:
	float m_kvEdge = 0.2f;
	float m_kvGain = 0.05f;
	float m_kvLowpassCutoff = 8'000.f;
	float m_kvEqCenter = 3'600.f;
	float m_kvEqBandwidth = 3'600.f;
	BaseEnvSoundDspDistortion();
};

#endif