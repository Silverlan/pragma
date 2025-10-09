// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "sharedutils/util_event_reply.hpp"

#include "stdafx_client.h"

module pragma.client;

import :gui.checkbox;
import :gui.silk_icon;

import :client_state;

WICheckbox::WICheckbox() : WIRect(), m_bChecked(false) { RegisterCallback<void, bool>("OnChange"); }

WICheckbox::~WICheckbox() {}

void WICheckbox::Initialize()
{
	WIRect::Initialize();

	m_hOutline = CreateChild<WIOutlinedRect>();
	WIOutlinedRect *pOutline = m_hOutline.get<WIOutlinedRect>();
	pOutline->SetColor(0.f, 0.f, 0.f, 1.f);
	pOutline->SetOutlineWidth(1);

	m_hTick = CreateChild<WISilkIcon>();
	WISilkIcon *pIcon = m_hTick.get<WISilkIcon>();
	pIcon->SetIcon("tick");
	pIcon->SetPos(1, 1);
	pIcon->SetVisible(false);

	SetSize(18, 18);
	SetMouseInputEnabled(true);
}
void WICheckbox::SetChecked(bool bChecked)
{
	m_bChecked = bChecked;
	if(m_hTick.IsValid())
		m_hTick->SetVisible(bChecked);
	CallCallbacks<void, bool>("OnChange", bChecked);
}
bool WICheckbox::IsChecked() { return m_bChecked; }
void WICheckbox::Toggle() { SetChecked(!m_bChecked); }
util::EventReply WICheckbox::MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	if(WIRect::MouseCallback(button, state, mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(button == pragma::platform::MouseButton::Left && state == pragma::platform::KeyState::Press)
		Toggle();
	return util::EventReply::Handled;
}
void WICheckbox::SetSize(int x, int y)
{
	WIRect::SetSize(x, y);
	if(m_hOutline.IsValid()) {
		WIOutlinedRect *pOutline = m_hOutline.get<WIOutlinedRect>();
		pOutline->SetSize(x, y);
	}
}
