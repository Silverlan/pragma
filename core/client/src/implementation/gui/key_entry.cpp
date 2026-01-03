// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.key_entry;

// TODO
// Create full-screen element to capture mouse inputs
// Don't allow same key for 'key 1' and 'key 2'
// Enable escape key (To undo binding)

pragma::gui::types::WIKeyEntry::WIKeyEntry() : WITextEntryBase(), m_bKeyPressed(false), m_key(static_cast<platform::Key>(-1)) { RegisterCallback<void, platform::Key, platform::Key>("OnKeyChanged"); }

pragma::gui::types::WIKeyEntry::~WIKeyEntry()
{
	if(m_hMouseTrap.IsValid())
		m_hMouseTrap->Remove();
}

void pragma::gui::types::WIKeyEntry::OnTextChanged(const string::Utf8String &text, bool changedByUser)
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

void pragma::gui::types::WIKeyEntry::Initialize()
{
	WITextEntryBase::Initialize();
	SetHeight(24);
	if(m_hCaret.IsValid())
		m_hCaret->Remove();
	if(m_hText.IsValid()) {
		auto hThis = GetHandle();
		m_hText->AddCallback("OnTextChanged", FunctionCallback<void, std::reference_wrapper<const string::Utf8String>>::Create([hThis](std::reference_wrapper<const string::Utf8String> newText) mutable {
			if(!hThis.IsValid())
				return;
			static_cast<WIKeyEntry *>(hThis.get())->OnTextChanged(newText, false);
		}));
	}
	ApplyKey(static_cast<platform::Key>(-1));
}

void pragma::gui::types::WIKeyEntry::SetSize(int x, int y)
{
	WITextEntryBase::SetSize(x, y);
	if(!m_hText.IsValid())
		return;
	WIText *t = static_cast<WIText *>(m_hText.get());
	OnTextChanged(t->GetText(), false);
}

pragma::util::EventReply pragma::gui::types::WIKeyEntry::KeyboardCallback(platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods)
{
	if(WIBase::KeyboardCallback(key, scanCode, state, mods) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(state != platform::KeyState::Press)
		return util::EventReply::Handled;
	ApplyKey(key);
	return util::EventReply::Handled;
}
pragma::util::EventReply pragma::gui::types::WIKeyEntry::ScrollCallback(Vector2 offset, bool offsetAsPixels)
{
	if(WIBase::ScrollCallback(offset, offsetAsPixels) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(offset.y >= 0.f)
		ApplyKey(static_cast<platform::Key>(GLFW_CUSTOM_KEY_SCRL_UP));
	else
		ApplyKey(static_cast<platform::Key>(GLFW_CUSTOM_KEY_SCRL_DOWN));
	return util::EventReply::Handled;
}
void pragma::gui::types::WIKeyEntry::SetKey(platform::Key key)
{
	if(key == m_key)
		return;
	ApplyKey(key);
}
pragma::platform::Key pragma::gui::types::WIKeyEntry::GetKey() const { return m_key; }
void pragma::gui::types::WIKeyEntry::ApplyKey(platform::Key key)
{
	auto prevKey = m_key;
	if(math::to_integral(key) >= 'A' && math::to_integral(key) <= 'Z')
		key = static_cast<platform::Key>(tolower(math::to_integral(key)));
	m_key = key;
	std::string skey;
	if(key == static_cast<platform::Key>(-1))
		skey = locale::get_text("not_assigned");
	else if(key != platform::Key::Escape)
		KeyToText(CInt16(key), &skey);
	//std::transform(skey.begin(),skey.end(),skey.begin(),::tolower);
	SetText(skey);
	m_bKeyPressed = true;
	KillFocus();
	CallCallbacks<void, platform::Key, platform::Key>("OnKeyChanged", prevKey, key);
}
pragma::util::EventReply pragma::gui::types::WIKeyEntry::CharCallback(unsigned int c, platform::Modifier mods) { return WIBase::CharCallback(c, mods); }
void pragma::gui::types::WIKeyEntry::OnFocusGained()
{
	WITextEntryBase::OnFocusGained();
	if(m_hMouseTrap.IsValid())
		m_hMouseTrap->Remove();
	m_previousKey = GetText().cpp_str();
	SetText(locale::get_text("press_a_key"));
	auto *pRect = WGUI::GetInstance().Create<WIBase>();
	m_hMouseTrap = pRect->GetHandle();
	pRect->SetAutoAlignToParent(true);
	pRect->SetZPos(10'000);
	pRect->SetMouseInputEnabled(true);
	pRect->SetScrollInputEnabled(true);
	auto hKeyEntry = GetHandle();
	auto hRect = pRect->GetHandle();
	pRect->AddCallback("OnMouseEvent",
	  FunctionCallback<util::EventReply, platform::MouseButton, platform::KeyState, platform::Modifier>::CreateWithOptionalReturn([hRect, hKeyEntry](util::EventReply *reply, platform::MouseButton button, platform::KeyState state, platform::Modifier) mutable -> CallbackReturnType {
		  if(state != platform::KeyState::Press || !hKeyEntry.IsValid()) {
			  *reply = util::EventReply::Handled;
			  return CallbackReturnType::HasReturnValue;
		  }
		  auto *pKeyEntry = static_cast<WIKeyEntry *>(hKeyEntry.get());
		  pKeyEntry->ApplyKey(static_cast<platform::Key>(static_cast<uint32_t>(button) + static_cast<uint32_t>(platform::Key::Last)));
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
		pKeyEntry->ApplyKey(static_cast<platform::Key>((offset.y >= 0.f) ? GLFW_CUSTOM_KEY_SCRL_UP : GLFW_CUSTOM_KEY_SCRL_DOWN));
		if(hRect.IsValid())
			hRect.get()->RemoveSafely();
		*reply = util::EventReply::Handled;
		return CallbackReturnType::HasReturnValue;
	}));
	m_bKeyPressed = false;
}
void pragma::gui::types::WIKeyEntry::OnFocusKilled()
{
	WITextEntryBase::OnFocusKilled();
	if(m_bKeyPressed == false)
		SetText(m_previousKey);
	m_previousKey = "";
	if(m_hMouseTrap.IsValid())
		m_hMouseTrap->Remove();
}
