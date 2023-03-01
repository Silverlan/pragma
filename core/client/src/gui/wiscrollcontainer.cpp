/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/gui/wiscrollcontainer.h"
#include <wgui/types/wiscrollbar.h>
#include <mathutil/umath.h>

LINK_WGUI_TO_CLASS(WIScrollContainer, WIScrollContainer);
WIScrollContainer::WIScrollContainer() : WIBase() { SetScrollInputEnabled(true); }

WIScrollContainer::~WIScrollContainer()
{
	if(m_hChildAdded.IsValid())
		m_hChildAdded.Remove();
	if(m_hChildRemoved.IsValid())
		m_hChildRemoved.Remove();
	std::unordered_map<WIBase *, std::vector<CallbackHandle>>::iterator it;
	for(it = m_childCallbackHandles.begin(); it != m_childCallbackHandles.end(); it++) {
		std::vector<CallbackHandle> &handles = it->second;
		std::vector<CallbackHandle>::iterator itCb;
		for(itCb = handles.begin(); itCb != handles.end(); itCb++) {
			CallbackHandle &hCallback = *itCb;
			if(hCallback.IsValid())
				hCallback.Remove();
		}
	}
}

void WIScrollContainer::SizeToContents(bool x, bool y)
{
	if(m_hWrapper.IsValid() == false)
		return;
	m_hWrapper->SizeToContents(x, y);
	auto sz = m_hWrapper.get()->GetSize();
	SetSize(sz.x, sz.y);
}

int WIScrollContainer::GetContentWidth()
{
	auto w = GetWidth();
	if(m_hScrollBarV.IsValid() && m_hScrollBarV->IsVisible())
		w -= m_hScrollBarV->GetWidth();
	return w;
}

int WIScrollContainer::GetContentHeight()
{
	auto h = GetHeight();
	if(m_hScrollBarH.IsValid() && m_hScrollBarH->IsVisible())
		h -= m_hScrollBarH->GetHeight();
	return h;
}

Vector2i WIScrollContainer::GetContentSize()
{
	if(!m_hWrapper.IsValid())
		return Vector2i(0, 0);
	return Vector2i(GetContentWidth(), GetContentHeight());
}

WIBase *WIScrollContainer::GetWrapperElement() { return m_hWrapper.get(); }
void WIScrollContainer::ScrollToElement(::WIBase &el)
{
	ScrollToElementX(el);
	ScrollToElementY(el);
}
void WIScrollContainer::ScrollToElementX(::WIBase &el)
{
	auto *elWrapper = GetWrapperElement();
	auto *scrlBar = GetHorizontalScrollBar();
	if(!elWrapper || !scrlBar)
		return;
	auto offset = el.GetAbsolutePos().x - elWrapper->GetAbsolutePos().x;
	offset -= (GetWidth() + el.GetWidth()) / 2;
	scrlBar->SetScrollOffset(offset);
}
void WIScrollContainer::ScrollToElementY(::WIBase &el)
{
	auto *elWrapper = GetWrapperElement();
	auto *scrlBar = GetVerticalScrollBar();
	if(!elWrapper || !scrlBar)
		return;
	auto offset = el.GetAbsolutePos().y - elWrapper->GetAbsolutePos().y;
	offset -= (GetHeight() + el.GetHeight()) / 2;
	scrlBar->SetScrollOffset(offset);
}

void WIScrollContainer::SetAutoStickToBottom(bool autoStick) { umath::set_flag(m_scFlags, StateFlags::AutoStickToBottom, autoStick); }
bool WIScrollContainer::ShouldAutoStickToBottom() const { return umath::is_flag_set(m_scFlags, StateFlags::AutoStickToBottom); }

void WIScrollContainer::SetContentsWidthFixed(bool fixed) { umath::set_flag(m_scFlags, StateFlags::ContentsWidthFixed, fixed); }
void WIScrollContainer::SetContentsHeightFixed(bool fixed) { umath::set_flag(m_scFlags, StateFlags::ContentsHeightFixed, fixed); }
bool WIScrollContainer::IsContentsWidthFixed() const { return umath::is_flag_set(m_scFlags, StateFlags::ContentsWidthFixed); }
bool WIScrollContainer::IsContentsHeightFixed() const { return umath::is_flag_set(m_scFlags, StateFlags::ContentsHeightFixed); }

int WIScrollContainer::GetScrollBarWidthV()
{
	if(!m_hScrollBarV.IsValid() || !m_hScrollBarV->IsVisible())
		return 0;
	return m_hScrollBarV->GetWidth();
}
int WIScrollContainer::GetScrollBarHeightH()
{
	if(!m_hScrollBarH.IsValid() || !m_hScrollBarH->IsVisible())
		return 0;
	return m_hScrollBarH->GetHeight();
}

void WIScrollContainer::OnHScrollOffsetChanged(unsigned int offset)
{
	if(!m_hWrapper.IsValid() || offset == CUInt32(m_lastOffset.x))
		return;
	WIBase *pWrapper = m_hWrapper.get();
	pWrapper->SetX(pWrapper->GetX() - (offset - m_lastOffset.x));
	m_lastOffset.x = offset;
}
void WIScrollContainer::OnVScrollOffsetChanged(unsigned int offset)
{
	if(!m_hWrapper.IsValid() || offset == CUInt32(m_lastOffset.y))
		return;
	WIBase *pWrapper = m_hWrapper.get();
	pWrapper->SetY(pWrapper->GetY() - (offset - m_lastOffset.y));
	m_lastOffset.y = offset;
}

void WIScrollContainer::Initialize()
{
	WIBase::Initialize();
	m_hScrollBarH = CreateChild<WIScrollBar>();
	WIScrollBar *pScrollBarH = m_hScrollBarH.get<WIScrollBar>();
	pScrollBarH->SetVisible(false);
	pScrollBarH->SetZPos(1000);
	pScrollBarH->AddStyleClass("scrollbar_horizontal");
	pScrollBarH->AddCallback("OnScrollOffsetChanged", FunctionCallback<void, unsigned int>::Create(std::bind(&WIScrollContainer::OnHScrollOffsetChanged, this, std::placeholders::_1)));

	m_hScrollBarV = CreateChild<WIScrollBar>();
	WIScrollBar *pScrollBarV = m_hScrollBarV.get<WIScrollBar>();
	pScrollBarV->SetVisible(false);
	pScrollBarV->SetZPos(1000);
	pScrollBarH->AddStyleClass("scrollbar_vertical");
	pScrollBarV->AddCallback("OnScrollOffsetChanged", FunctionCallback<void, unsigned int>::Create(std::bind(&WIScrollContainer::OnVScrollOffsetChanged, this, std::placeholders::_1)));

	m_hWrapper = CreateChild<WIBase>();
	WIBase *pWrapper = m_hWrapper.get();

	// pWrapper->SetAutoAlignToParent(true);
	m_hChildAdded = pWrapper->AddCallback("OnChildAdded", FunctionCallback<void, WIBase *>::Create(std::bind(&WIScrollContainer::OnWrapperChildAdded, this, std::placeholders::_1)));
	m_hChildRemoved = pWrapper->AddCallback("OnChildRemoved", FunctionCallback<void, WIBase *>::Create(std::bind(&WIScrollContainer::OnWrapperChildRemoved, this, std::placeholders::_1)));
}
util::EventReply WIScrollContainer::ScrollCallback(Vector2 offset)
{
	if(WIBase::ScrollCallback(offset) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(m_hScrollBarH.IsValid())
		m_hScrollBarH.get<WIScrollBar>()->ScrollCallback(Vector2(offset.x, 0.f));
	if(m_hScrollBarV.IsValid())
		m_hScrollBarV.get<WIScrollBar>()->ScrollCallback(Vector2(0.f, offset.y));
	return util::EventReply::Handled;
}
WIScrollBar *WIScrollContainer::GetHorizontalScrollBar() { return static_cast<WIScrollBar *>(m_hScrollBarH.get()); }
WIScrollBar *WIScrollContainer::GetVerticalScrollBar() { return static_cast<WIScrollBar *>(m_hScrollBarV.get()); }
void WIScrollContainer::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);
	ScheduleUpdate();
}
void WIScrollContainer::OnChildReleased(WIBase *child)
{
	WIScrollContainer *parent = dynamic_cast<WIScrollContainer *>(child->GetParent());
	if(parent == nullptr)
		return;
	parent->OnChildRemoved(child);
}
void WIScrollContainer::OnChildSetSize(WIBase *child)
{
	WIBase *parentWrapper = child->GetParent();
	if(parentWrapper == nullptr)
		return;
	WIScrollContainer *parent = dynamic_cast<WIScrollContainer *>(parentWrapper->GetParent());
	if(parent == nullptr)
		return;
	parent->ScheduleUpdate();
}
void WIScrollContainer::OnWrapperChildAdded(WIBase *child)
{
	std::unordered_map<WIBase *, std::vector<CallbackHandle>>::iterator it = m_childCallbackHandles.insert(std::unordered_map<WIBase *, std::vector<CallbackHandle>>::value_type(child, std::vector<CallbackHandle>())).first;
	CallbackHandle hCallbackOnRemove = child->AddCallback("OnRemove", FunctionCallback<>::Create(std::bind(&WIScrollContainer::OnChildReleased, child)));
	it->second.push_back(hCallbackOnRemove);
	CallbackHandle hCallbackSetSize = child->AddCallback("SetSize", FunctionCallback<>::Create(std::bind(&WIScrollContainer::OnChildSetSize, child)));
	it->second.push_back(hCallbackSetSize);
	Update();
}
void WIScrollContainer::OnWrapperChildRemoved(WIBase *child)
{
	std::unordered_map<WIBase *, std::vector<CallbackHandle>>::iterator it = m_childCallbackHandles.find(child);
	if(it != m_childCallbackHandles.end()) {
		std::vector<CallbackHandle> &callbacks = it->second;
		std::vector<CallbackHandle>::iterator itCb;
		for(itCb = callbacks.begin(); itCb != callbacks.end(); itCb++) {
			CallbackHandle &hCallback = *itCb;
			if(hCallback.IsValid())
				hCallback.Remove();
		}
		m_childCallbackHandles.erase(it);
	}
	Update();
}
void WIScrollContainer::OnChildAdded(WIBase *child)
{
	WIBase::OnChildAdded(child);
	if(m_hWrapper.IsValid())
		child->SetParent(m_hWrapper.get());
}
void WIScrollContainer::OnChildRemoved(WIBase *child) { WIBase::OnChildRemoved(child); }

void WIScrollContainer::ScrollToBottom()
{
	WIScrollBar *pScrollBar = m_hScrollBarV.get<WIScrollBar>();
	if(pScrollBar == nullptr)
		return;
	pScrollBar->SetScrollOffset(pScrollBar->GetElementCount());
}

void WIScrollContainer::SetScrollAmount(int32_t amX, int32_t amY)
{
	SetScrollAmountX(amX);
	SetScrollAmountY(amY);
}
void WIScrollContainer::SetScrollAmountX(int32_t amX)
{
	m_scrollAmountX = amX;
	ScheduleUpdate();
}
void WIScrollContainer::SetScrollAmountY(int32_t amY)
{
	m_scrollAmountY = amY;
	ScheduleUpdate();
}

void WIScrollContainer::DoUpdate()
{
	if(m_hWrapper.IsValid()) {
		WIBase *pWrapper = m_hWrapper.get();
		int w = 0;
		int h = 0;
		std::vector<WIHandle> *children = pWrapper->GetChildren();
		std::vector<WIHandle>::iterator it;
		for(it = children->begin(); it != children->end(); it++) {
			WIHandle &hChild = *it;
			if(hChild.IsValid() == false || hChild->IsBackgroundElement())
				continue;
			const Vector2i &posChild = hChild->GetPos();
			const Vector2i &sizeChild = hChild->GetSize();
			if(posChild.x + sizeChild.x > w)
				w = posChild.x + sizeChild.x;
			if(posChild.y + sizeChild.y > h)
				h = posChild.y + sizeChild.y;
		}
		Vector2i size = GetSize();
		if(IsContentsWidthFixed())
			w = size.x;
		if(IsContentsHeightFixed())
			h = size.y;
		pWrapper->SetSize(w, h);
		if(m_hScrollBarH.IsValid()) {
			WIScrollBar *pScrollBar = m_hScrollBarH.get<WIScrollBar>();
			pScrollBar->SetUp(size.x, w);
			pScrollBar->SetScrollAmount(m_scrollAmountX);
			pScrollBar->SetSize(GetWidth(), 10);
			pScrollBar->SetY(GetHeight() - pScrollBar->GetHeight());
			pScrollBar->SetVisible(w > size.x); // set scroll offset to 0
		}
		if(m_hScrollBarV.IsValid()) {
			WIScrollBar *pScrollBar = m_hScrollBarV.get<WIScrollBar>();
			auto bAtBottom = pScrollBar->GetBottomScrollOffset() == pScrollBar->GetElementCount();

			pScrollBar->SetUp(size.y, h);
			pScrollBar->SetScrollAmount(m_scrollAmountY);
			pScrollBar->SetSize(10, GetHeight());
			pScrollBar->SetX(GetWidth() - pScrollBar->GetWidth());

			if(bAtBottom == true && ShouldAutoStickToBottom())
				ScrollToBottom();
			pScrollBar->SetVisible(h > size.y);
		}
	}
	WIBase::DoUpdate();
}
