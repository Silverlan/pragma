/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_eye_component.hpp"
#include "pragma/entities/components/c_flex_component.hpp"
#include "pragma/model/c_model.h"

extern DLLCLIENT CGame *c_game;

using namespace pragma;

void CEyeComponent::Blink()
{
	m_tNextBlink = c_game->CurTime() + umath::random(1.5f, 4.f);
	umath::set_flag(m_stateFlags, StateFlags::BlinkToggle, !umath::is_flag_set(m_stateFlags, StateFlags::BlinkToggle));
}
void CEyeComponent::UpdateBlinkMT()
{
	if(m_blinkFlexController == std::numeric_limits<uint32_t>::max() || IsBlinkingEnabled() == false || m_flexC.expired())
		return;

	// check for blinking
	if(umath::is_flag_set(m_stateFlags, StateFlags::BlinkToggle) != umath::is_flag_set(m_stateFlags, StateFlags::PrevBlinkToggle)) {
		umath::set_flag(m_stateFlags, StateFlags::PrevBlinkToggle, umath::is_flag_set(m_stateFlags, StateFlags::BlinkToggle));

		m_curBlinkTime = c_game->CurTime() + GetBlinkDuration();
		// BroadcastEvent(EVENT_ON_BLINK);
	}
	auto blinkFlexControllerWeight = 0.f;

	// blink the eyes
	auto t = (m_curBlinkTime - c_game->CurTime()) * umath::pi * 0.5f * (1.0 / m_blinkDuration);
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

	auto tCur = c_game->CurTime();
	if(tCur >= m_tNextBlink)
		Blink();
}
