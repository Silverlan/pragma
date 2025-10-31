// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

module pragma.client;


import :entities.components.eye;
import :game;


using namespace pragma;

void CEyeComponent::Blink()
{
	m_tNextBlink = pragma::get_cgame()->CurTime() + umath::random(1.5f, 4.f);
	umath::set_flag(m_stateFlags, StateFlags::BlinkToggle, !umath::is_flag_set(m_stateFlags, StateFlags::BlinkToggle));
}
void CEyeComponent::UpdateBlinkMT()
{
	if(m_blinkFlexController == std::numeric_limits<uint32_t>::max() || IsBlinkingEnabled() == false || m_flexC.expired())
		return;

	// check for blinking
	if(umath::is_flag_set(m_stateFlags, StateFlags::BlinkToggle) != umath::is_flag_set(m_stateFlags, StateFlags::PrevBlinkToggle)) {
		umath::set_flag(m_stateFlags, StateFlags::PrevBlinkToggle, umath::is_flag_set(m_stateFlags, StateFlags::BlinkToggle));

		m_curBlinkTime = pragma::get_cgame()->CurTime() + GetBlinkDuration();
		// BroadcastEvent(EVENT_ON_BLINK);
	}
	auto blinkFlexControllerWeight = 0.f;

	// blink the eyes
	auto t = (m_curBlinkTime - pragma::get_cgame()->CurTime()) * umath::pi * 0.5f * (1.0 / m_blinkDuration);
	if(t > 0) {
		// do eyeblink falloff curve
		t = umath::cos(t);
		if(t > 0) {
			blinkFlexControllerWeight = umath::sqrt(t) * 2.f;
			if(blinkFlexControllerWeight > 1)
				blinkFlexControllerWeight = 2.f - blinkFlexControllerWeight;
		}
	}
	m_flexC->SetFlexController(m_blinkFlexController, blinkFlexControllerWeight);

	auto tCur = pragma::get_cgame()->CurTime();
	if(tCur >= m_tNextBlink)
		Blink();
}
