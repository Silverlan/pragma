/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/gui/wiframe.h"
#include "pragma/gui/widetachable.hpp"
#include <wgui/types/witext.h>
#include <wgui/types/wibutton.h>
#include <wgui/types/wirect.h>
#include <mathutil/umath.h>
#include <prosper_window.hpp>

import pragma.string.unicode;

LINK_WGUI_TO_CLASS(WIFrame, WIFrame);

#define WIFRAME_DRAG_OFFSET_BORDER 5
#define WIFRAME_RESIZE_OFFSET_BORDER 5

WIFrame::WIFrame() : WITransformable() {}
WIFrame::~WIFrame()
{
	if(m_resizeMode != ResizeMode::none)
		WGUI::GetInstance().SetCursor(GLFW::Cursor::Shape::Arrow);
}

WIBase *WIFrame::GetContents() { return m_hContents.get(); }

void WIFrame::OnDetachButtonPressed() { Detach(); }

void WIFrame::Initialize()
{
	SetDraggable(true);
	SetResizable(true);
	SetSize(256, 256);

	m_hBg = CreateChild<WIRect>();
	m_hBg->SetName("background");
	m_hBg->SetAutoAlignToParent(true);
	m_hBg->GetColorProperty()->Link(*GetColorProperty());

	m_hContents = CreateChild<WIDetachable>();
	m_hContents->SetY(30);
	m_hContents->SetSize(GetWidth(), GetHeight() - m_hContents->GetY());
	m_hContents->SetAnchor(0.f, 0.f, 1.f, 1.f);

	WITransformable::Initialize();
	if(m_hMoveRect.IsValid()) {
		auto &gui = WGUI::GetInstance();
		m_hTitleBar = gui.Create<WIBase>(m_hMoveRect.get())->GetHandle();
		WIBase *pTitleBar = m_hTitleBar.get();
		pTitleBar->AddStyleClass("frame_titlebar");
		pTitleBar->SetAutoAlignToParent(true);
		auto hFrame = GetHandle();
		pTitleBar->AddCallback("SetSize", FunctionCallback<>::Create([hFrame]() mutable {
			if(!hFrame.IsValid())
				return;
			auto *pFrame = static_cast<WIFrame *>(hFrame.get());
			if(!pFrame->m_hTitleBar.IsValid())
				return;
			auto *pTitleBar = pFrame->m_hTitleBar.get();
			if(pFrame->m_hClose.IsValid()) {
				WIButton *pButton = static_cast<WIButton *>(pFrame->m_hClose.get());
				pButton->SetX(pTitleBar->GetWidth() - pButton->GetWidth() - 10);
				pButton->SetY(CInt32(pTitleBar->GetHeight() * 0.5f - pButton->GetHeight() * 0.5f));
			}
			if(pFrame->m_hClose.IsValid() && pFrame->m_hDetachButton.IsValid()) {
				WIButton *pButton = static_cast<WIButton *>(pFrame->m_hDetachButton.get());
				pButton->SetX(pFrame->m_hClose->GetX() - pButton->GetWidth() - 5);
				pButton->SetY(pFrame->m_hClose->GetY());
			}
			if(pFrame->m_hTitle.IsValid()) {
				WIText *pText = static_cast<WIText *>(pFrame->m_hTitle.get());
				pText->SetX(10);
				pText->SetY(CInt32(pTitleBar->GetHeight() * 0.5f - pText->GetHeight() * 0.5f));
			}
		}));

		m_hTitle = gui.Create<WIText>(pTitleBar)->GetHandle();
		WIText *pTitle = static_cast<WIText *>(m_hTitle.get());
		pTitle->AddStyleClass("frame_title");
		pTitle->SetName("frame_title");
		if(pTitle != nullptr)
			pTitle->SetVisible(false);

		{
			m_hDetachButton = gui.Create<WIButton>(pTitleBar)->GetHandle();
			WIButton *pButton = static_cast<WIButton *>(m_hDetachButton.get());
			pButton->SetText(".");
			pButton->AddCallback("OnPressed", FunctionCallback<util::EventReply>::CreateWithOptionalReturn([this](util::EventReply *reply) -> CallbackReturnType {
				*reply = util::EventReply::Handled;
				OnDetachButtonPressed();
				return CallbackReturnType::HasReturnValue;
			}));
			pButton->SetSize(20, 20);
		}

		{
			m_hClose = gui.Create<WIButton>(pTitleBar)->GetHandle();
			WIButton *pButton = static_cast<WIButton *>(m_hClose.get());
			pButton->SetText("X");
			pButton->AddCallback("OnPressed", FunctionCallback<util::EventReply>::CreateWithOptionalReturn([this](util::EventReply *reply) -> CallbackReturnType {
				*reply = util::EventReply::Handled;
				OnCloseButtonPressed();
				return CallbackReturnType::HasReturnValue;
			}));
			pButton->SetSize(20, 20);
		}
	}
}
util::EventReply WIFrame::MouseCallback(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods)
{
	if(WITransformable::MouseCallback(button, state, mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	RequestFocus();
	return util::EventReply::Handled;
}
void WIFrame::SetCloseButtonEnabled(bool b)
{
	if(!m_hClose.IsValid())
		return;
	m_hClose->SetVisible(b);
}
bool WIFrame::IsDetached() const
{
	auto *detachable = dynamic_cast<const WIDetachable *>(m_hContents.get());
	if(!detachable)
		return false;
	return detachable->IsDetached();
}
void WIFrame::Detach()
{
	auto *detachable = dynamic_cast<WIDetachable *>(m_hContents.get());
	if(!detachable)
		return;
	detachable->Detach();
}
void WIFrame::Reattach()
{
	auto *detachable = dynamic_cast<WIDetachable *>(m_hContents.get());
	if(!detachable)
		return;
	detachable->Reattach();
}
void WIFrame::SetDetachButtonEnabled(bool b)
{
	if(!m_hDetachButton.IsValid())
		return;
	m_hDetachButton->SetVisible(b);
}
void WIFrame::SetSize(int x, int y)
{
	WITransformable::SetSize(x, y);
	if(m_hMoveRect.IsValid())
		m_hMoveRect->SetWidth(x);
	Vector2i minSize = m_minSize;
	if(x < minSize.x || y < minSize.y) {
		if(x < minSize.x)
			minSize.x = x;
		if(y < minSize.y)
			minSize.y = y;
		SetMinSize(minSize);
	}
	Vector2i maxSize = m_maxSize;
	if((x > maxSize.x && maxSize.x != -1) || (y > maxSize.y && maxSize.y != -1)) {
		if(x > maxSize.x && maxSize.x != -1)
			maxSize.x = x;
		if(y > maxSize.y && maxSize.y != -1)
			maxSize.y = y;
		SetMaxSize(maxSize);
	}
}

void WIFrame::SetTitle(std::string title)
{
	if(!m_hTitle.IsValid())
		return;
	WIText *pText = static_cast<WIText *>(m_hTitle.get());
	pText->SetText(title);
	pText->SizeToContents();
	pText->SetVisible(!title.empty());
}
const pragma::string::Utf8String &WIFrame::GetTitle() const
{
	if(!m_hTitle.IsValid()) {
		static pragma::string::Utf8String emptyString {};
		return emptyString;
	}
	return static_cast<const WIText *>(m_hTitle.get())->GetText();
}
