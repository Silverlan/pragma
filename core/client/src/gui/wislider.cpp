/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/gui/wislider.h"
#include <wgui/types/witext.h>
#include <wgui/types/wibutton.h>
#include "pragma/gui/wiprogressbar.h"

LINK_WGUI_TO_CLASS(WISlider, WISlider);

WISlider::WISlider() : WIProgressBar(), m_bMoveSlider(false) { SetMouseInputEnabled(true); }

WISlider::~WISlider() {}

void WISlider::Initialize()
{
	WIProgressBar::Initialize();
	EnableThinking();
}

void WISlider::SetSize(int x, int y) { WIProgressBar::SetSize(x, y); }

void WISlider::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	WIProgressBar::Think(drawCmd);
	if(m_bMoveSlider == false)
		return;
	int x;
	GetMousePos(&x, nullptr);
	auto width = GetWidth();
	float v = CFloat(x) / CFloat(width);
	SetProgress(v);
}

bool WISlider::IsBeingDragged() const { return m_bMoveSlider; }

util::EventReply WISlider::MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	if(WIProgressBar::MouseCallback(button, state, mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(button == pragma::platform::MouseButton::Left) {
		if(state == pragma::platform::KeyState::Press)
			m_bMoveSlider = true;
		else
			m_bMoveSlider = false;
	}
	return util::EventReply::Handled;
}
