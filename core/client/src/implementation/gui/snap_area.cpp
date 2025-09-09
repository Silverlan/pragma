// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include <wgui/types/witext.h>

module pragma.client.gui;

import :snap_area;

extern CEngine *c_engine;
extern ClientState *client;

LINK_WGUI_TO_CLASS(WISnapArea, WISnapArea);

WISnapArea::WISnapArea() : WIBase() {}

void WISnapArea::Initialize()
{
	WIBase::Initialize();
	SetSize(50, 50);
	auto *pTriggerArea = WGUI::GetInstance().Create<WIBase>(this);
	pTriggerArea->SetSize(GetSize());
	pTriggerArea->SetAnchor(0.f, 0.f, 1.f, 1.f);
	m_hTriggerArea = pTriggerArea->GetHandle();
}

WIBase *WISnapArea::GetTriggerArea() { return m_hTriggerArea.get(); }
