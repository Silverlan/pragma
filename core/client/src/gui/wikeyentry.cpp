/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/gui/wikeyentry.h"
#include <wgui/types/witext.h>
#include <wgui/types/wirect.h>
#include "pragma/input/inputhelper.h"
#include "pragma/localization.h"
#include <mathutil/umath.h>

// TODO
// Create full-screen element to capture mouse inputs
// Don't allow same key for 'key 1' and 'key 2'
// Enable escape key (To undo binding)

WIKeyEntry::WIKeyEntry() : WITextEntryBase(), m_bKeyPressed(false), m_key(static_cast<GLFW::Key>(-1)) { RegisterCallback<void, GLFW::Key, GLFW::Key>("OnKeyChanged"); }

WIKeyEntry::~WIKeyEntry()
{
	if(m_hMouseTrap.IsValid())
		m_hMouseTrap->Remove();
}

void WIKeyEntry::OnTextChanged(const pragma::string::Utf8String &text, bool changedByUser)
{
	WITextEntryBase::OnTextChanged(text, changedByUser);
	if(!m_hText.IsValid())
		return;
	WIText *t = static_cast<WIText *>(m_hText.get());
	t->SizeToContents();
	int w = GetWidth();
	int wText = t->GetWidth();
	t->SetX(CInt32(w * 0.5f - wText * 0.5f));
	t->CenterToParentY();
}

void WIKeyEntry::Initialize()
{
	WITextEntryBase::Initialize();
	SetHeight(24);
	if(m_hCaret.IsValid())
		m_hCaret->Remove();
	if(m_hText.IsValid()) {
		auto hThis = GetHandle();
		m_hText->AddCallback("OnTextChanged", FunctionCallback<void, std::reference_wrapper<const pragma::string::Utf8String>>::Create([hThis](std::reference_wrapper<const pragma::string::Utf8String> newText) mutable {
			if(!hThis.IsValid())
				return;
			static_cast<WIKeyEntry *>(hThis.get())->OnTextChanged(newText, false);
		}));
	}
	ApplyKey(static_cast<GLFW::Key>(-1));
}

void WIKeyEntry::SetSize(int x, int y)
{
	WITextEntryBase::SetSize(x, y);
	if(!m_hText.IsValid())
		return;
	WIText *t = static_cast<WIText *>(m_hText.get());
	OnTextChanged(t->GetText(), false);
}

util::EventReply WIKeyEntry::KeyboardCallback(GLFW::Key key, int scanCode, GLFW::KeyState state, GLFW::Modifier mods)
{
	if(WIBase::KeyboardCallback(key, scanCode, state, mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(state != GLFW::KeyState::Press)
		return util::EventReply::Handled;
	ApplyKey(key);
	return util::EventReply::Handled;
}
util::EventReply WIKeyEntry::ScrollCallback(Vector2 offset, bool offsetAsPixels)
{
	if(WIBase::ScrollCallback(offset, offsetAsPixels) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(offset.y >= 0.f)
		ApplyKey(static_cast<GLFW::Key>(GLFW_CUSTOM_KEY_SCRL_UP));
	else
		ApplyKey(static_cast<GLFW::Key>(GLFW_CUSTOM_KEY_SCRL_DOWN));
	return util::EventReply::Handled;
}
void WIKeyEntry::SetKey(GLFW::Key key)
{
	if(key == m_key)
		return;
	ApplyKey(key);
}
GLFW::Key WIKeyEntry::GetKey() const { return m_key; }
void WIKeyEntry::ApplyKey(GLFW::Key key)
{
	auto prevKey = m_key;
	if(umath::to_integral(key) >= 'A' && umath::to_integral(key) <= 'Z')
		key = static_cast<GLFW::Key>(tolower(umath::to_integral(key)));
	m_key = key;
	std::string skey;
	if(key == static_cast<GLFW::Key>(-1))
		skey = Locale::GetText("not_assigned");
	else if(key != GLFW::Key::Escape)
		KeyToText(CInt16(key), &skey);
	//std::transform(skey.begin(),skey.end(),skey.begin(),::tolower);
	SetText(skey);
	m_bKeyPressed = true;
	KillFocus();
	CallCallbacks<void, GLFW::Key, GLFW::Key>("OnKeyChanged", prevKey, key);
}
util::EventReply WIKeyEntry::CharCallback(unsigned int c, GLFW::Modifier mods) { return WIBase::CharCallback(c, mods); }
void WIKeyEntry::OnFocusGained()
{
	WITextEntryBase::OnFocusGained();
	if(m_hMouseTrap.IsValid())
		m_hMouseTrap->Remove();
	m_previousKey = GetText().cpp_str();
	SetText(Locale::GetText("press_a_key"));
	auto *pRect = WGUI::GetInstance().Create<WIBase>();
	m_hMouseTrap = pRect->GetHandle();
	pRect->SetAutoAlignToParent(true);
	pRect->SetZPos(10'000);
	pRect->SetMouseInputEnabled(true);
	pRect->SetScrollInputEnabled(true);
	auto hKeyEntry = GetHandle();
	auto hRect = pRect->GetHandle();
	pRect->AddCallback("OnMouseEvent", FunctionCallback<util::EventReply, GLFW::MouseButton, GLFW::KeyState, GLFW::Modifier>::CreateWithOptionalReturn([hRect, hKeyEntry](util::EventReply *reply, GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier) mutable -> CallbackReturnType {
		if(state != GLFW::KeyState::Press || !hKeyEntry.IsValid()) {
			*reply = util::EventReply::Handled;
			return CallbackReturnType::HasReturnValue;
		}
		auto *pKeyEntry = static_cast<WIKeyEntry *>(hKeyEntry.get());
		pKeyEntry->ApplyKey(static_cast<GLFW::Key>(static_cast<uint32_t>(button) + static_cast<uint32_t>(GLFW::Key::Last)));
		if(hRect.IsValid())
			hRect.get()->RemoveSafely();
		*reply = util::EventReply::Handled;
		return CallbackReturnType::HasReturnValue;
	}));
	pRect->AddCallback("OnScroll", FunctionCallback<util::EventReply, Vector2, bool>::CreateWithOptionalReturn([hRect, hKeyEntry](util::EventReply *reply, Vector2 offset, bool offsetAsPixels) mutable -> CallbackReturnType {
		if(!hKeyEntry.IsValid()) {
			*reply = util::EventReply::Handled;
			return CallbackReturnType::HasReturnValue;
		}
		auto *pKeyEntry = static_cast<WIKeyEntry *>(hKeyEntry.get());
		pKeyEntry->ApplyKey(static_cast<GLFW::Key>((offset.y >= 0.f) ? GLFW_CUSTOM_KEY_SCRL_UP : GLFW_CUSTOM_KEY_SCRL_DOWN));
		if(hRect.IsValid())
			hRect.get()->RemoveSafely();
		*reply = util::EventReply::Handled;
		return CallbackReturnType::HasReturnValue;
	}));
	m_bKeyPressed = false;
}
void WIKeyEntry::OnFocusKilled()
{
	WITextEntryBase::OnFocusKilled();
	if(m_bKeyPressed == false)
		SetText(m_previousKey);
	m_previousKey = "";
	if(m_hMouseTrap.IsValid())
		m_hMouseTrap->Remove();
}
