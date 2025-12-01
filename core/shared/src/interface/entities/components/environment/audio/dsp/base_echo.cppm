// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.audio.dsp.base_echo;

export class DLLNETWORK BaseEnvSoundDspEcho {
  protected:
	float m_kvDelay = 0.1f;
	float m_kvLRDelay = 0.1f;
	float m_kvDamping = 0.5f;
	float m_kvFeedback = 0.5f;
	float m_kvSpread = -1.f;
	BaseEnvSoundDspEcho();
};
