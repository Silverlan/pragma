// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :gui.checkbox;
import :gui.silk_icon;

import :client_state;

pragma::gui::WICheckbox::WICheckbox() : WIRect(), m_bChecked(false) { RegisterCallback<void, bool>("OnChange"); }

pragma::gui::WICheckbox::~WICheckbox() {}

void pragma::gui::WICheckbox::Initialize()
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
void pragma::gui::WICheckbox::SetChecked(bool bChecked)
{
	m_bChecked = bChecked;
	if(m_hTick.IsValid())
		m_hTick->SetVisible(bChecked);
	CallCallbacks<void, bool>("OnChange", bChecked);
}
bool pragma::gui::WICheckbox::IsChecked() { return m_bChecked; }
void pragma::gui::WICheckbox::Toggle() { SetChecked(!m_bChecked); }
util::EventReply pragma::gui::WICheckbox::MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	if(WIRect::MouseCallback(button, state, mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(button == pragma::platform::MouseButton::Left && state == pragma::platform::KeyState::Press)
		Toggle();
	return util::EventReply::Handled;
}
void pragma::gui::WICheckbox::SetSize(int x, int y)
{
	WIRect::SetSize(x, y);
	if(m_hOutline.IsValid()) {
		WIOutlinedRect *pOutline = m_hOutline.get<WIOutlinedRect>();
		pOutline->SetSize(x, y);
	}
}
