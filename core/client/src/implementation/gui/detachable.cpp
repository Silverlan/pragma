// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :gui.detachable;

import :engine;
import pragma.gui;

#undef CreateWindow

pragma::gui::types::WIDetachable::DetachedWindow::~DetachedWindow() { Clear(); }
void pragma::gui::types::WIDetachable::DetachedWindow::Clear()
{
	if(detachedBg.IsValid())
		detachedBg.Remove();
	auto w = window;
	window = nullptr;
	if(w)
		w->Close();
}

pragma::gui::types::WIDetachable::WIDetachable() : WIBase {} {}
pragma::gui::types::WIDetachable::~WIDetachable() {}

void pragma::gui::types::WIDetachable::OnRemove()
{
	Reattach();
	WIBase::OnRemove();
}

bool pragma::gui::types::WIDetachable::IsDetached() const { return m_detachedWindow != nullptr; }

void pragma::gui::types::WIDetachable::Detach()
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
	settings.title = locale::get_text("console");
	m_detachedWindow = std::unique_ptr<DetachedWindow> {new DetachedWindow {}};
	m_detachedWindow->window = get_cengine()->CreateWindow(settings);
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
void pragma::gui::types::WIDetachable::Reattach()
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
