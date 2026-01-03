// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.snap_area;

import :client_state;
import :engine;

pragma::gui::types::WISnapArea::WISnapArea() : WIBase() {}

void pragma::gui::types::WISnapArea::Initialize()
{
	WIBase::Initialize();
	SetSize(50, 50);
	auto *pTriggerArea = WGUI::GetInstance().Create<WIBase>(this);
	pTriggerArea->SetSize(GetSize());
	pTriggerArea->SetAnchor(0.f, 0.f, 1.f, 1.f);
	m_hTriggerArea = pTriggerArea->GetHandle();
}

pragma::gui::types::WIBase *pragma::gui::types::WISnapArea::GetTriggerArea() { return m_hTriggerArea.get(); }
