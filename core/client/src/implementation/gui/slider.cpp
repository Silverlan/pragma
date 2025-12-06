// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :gui.slider;
import :gui.progress_bar;

import pragma.gui;

pragma::gui::WISlider::WISlider() : WIProgressBar(), m_bMoveSlider(false) { SetMouseInputEnabled(true); }

pragma::gui::WISlider::~WISlider() {}

void pragma::gui::WISlider::Initialize()
{
	WIProgressBar::Initialize();
	EnableThinking();
}

void pragma::gui::WISlider::SetSize(int x, int y) { WIProgressBar::SetSize(x, y); }

void pragma::gui::WISlider::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
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

bool pragma::gui::WISlider::IsBeingDragged() const { return m_bMoveSlider; }

util::EventReply pragma::gui::WISlider::MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods)
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
