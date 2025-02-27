/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/gui/widetachable.hpp"
#include <wgui/types/witext.h>
#include <wgui/types/wibutton.h>
#include <wgui/types/wirect.h>
#include <wgui/types/wiroot.h>
#include <mathutil/umath.h>
#include <prosper_window.hpp>

import pragma.locale;

extern DLLCLIENT CEngine *c_engine;

LINK_WGUI_TO_CLASS(WIDetachable, WIDetachable);

WIDetachable::DetachedWindow::~DetachedWindow() { Clear(); }
void WIDetachable::DetachedWindow::Clear()
{
	if(detachedBg.IsValid())
		detachedBg.Remove();
	auto w = window;
	window = nullptr;
	if(w)
		w->Close();
}

WIDetachable::WIDetachable() : WIBase {} {}
WIDetachable::~WIDetachable() {}

void WIDetachable::OnRemove()
{
	Reattach();
	WIBase::OnRemove();
}

bool WIDetachable::IsDetached() const { return m_detachedWindow != nullptr; }

void WIDetachable::Detach()
{
	// if(m_mode == Mode::SimplifiedOverlay)
	// 	SetSimpleConsoleMode(false);
	if(m_detachedWindow)
		return;
	auto *frame = GetParent();
	if(!frame)
		return;
	auto absPos = GetAbsolutePos();
	auto w = GetWidth();
	auto h = GetHeight();
	prosper::WindowSettings settings {};
	settings.width = w;
	settings.height = h;
	settings.title = pragma::locale::get_text("console");
	m_detachedWindow = std::unique_ptr<DetachedWindow> {new DetachedWindow {}};
	m_detachedWindow->window = c_engine->CreateWindow(settings);
	if(!m_detachedWindow->window)
		return;
	CallCallbacks("OnDetaching");
	m_detachedWindow->reattachElement = frame->GetHandle();
	m_detachedWindow->origPos = GetPos();
	m_detachedWindow->origSize = GetSize();
	if(HasAnchor()) {
		m_detachedWindow->origAnchor = std::array<float, 4> {};
		auto &anchor = *m_detachedWindow->origAnchor;
		GetAnchor(anchor[0], anchor[1], anchor[2], anchor[3]);
	}
	m_detachedWindow->window->AddCloseListener([this]() { Reattach(); });
	auto *elBase = WGUI::GetInstance().GetBaseElement(m_detachedWindow->window.get());
	assert(elBase);
	TrapFocus(false);
	KillFocus();
	if(elBase) {
		auto *elBg = WGUI::GetInstance().Create<WIRect>(elBase);
		assert(elBg);
		elBg->GetColorProperty()->Link(*frame->GetColorProperty());
		elBg->SetSize(w, h);
		elBg->SetAnchor(0, 0, 1, 1);

		ClearAnchor();
		SetParentAndUpdateWindow(elBg);
	}

	SetPos(0, 0);
	SetSize(w, h);
	SetAnchor(0, 0, 1, 1);
	TrapFocus(true);
	RequestFocus();

	frame->SetVisible(false);
	(*m_detachedWindow->window)->SetPos(absPos);
	CallCallbacks("OnDetached");
	// m_mode = Mode::ExternalWindow;
}
void WIDetachable::Reattach()
{
	if(!m_detachedWindow)
		return;
	CallCallbacks("OnReattaching");
	auto info = std::move(m_detachedWindow);
	m_detachedWindow = nullptr;
	auto *frame = info->reattachElement.get();
	if(frame) {
		TrapFocus(false);
		SetParentAndUpdateWindow(frame);
		ClearAnchor();
		SetPos(info->origPos);
		SetSize(info->origSize);
		if(info->origAnchor.has_value()) {
			auto &anchor = *info->origAnchor;
			SetAnchor(anchor[0], anchor[1], anchor[2], anchor[3]);
		}
		frame->SetVisible(true);
		RequestFocus();
	}
	info->Clear();
	CallCallbacks("OnReattached");
	// m_mode = Mode::Standard;
}
