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

WIKeyEntry::WIKeyEntry()
	: WITextEntryBase(),m_bKeyPressed(false),m_key(static_cast<GLFW::Key>(-1))
{
	RegisterCallback<void,GLFW::Key,GLFW::Key>("OnKeyChanged");
}

WIKeyEntry::~WIKeyEntry()
{
	if(m_hMouseTrap.IsValid())
		m_hMouseTrap->Remove();
}

void WIKeyEntry::OnTextChanged(const std::string &oldText,const std::string &text)
{
	WITextEntryBase::OnTextChanged(oldText,text);
	if(!m_hText.IsValid())
		return;
	WIText *t = m_hText.get<WIText>();
	t->SizeToContents();
	int w = GetWidth();
	int wText = t->GetWidth();
	t->SetX(CInt32(w *0.5f -wText *0.5f));
	t->CenterToParentY();
}

void WIKeyEntry::Initialize()
{
	WITextEntryBase::Initialize();
	SetHeight(24);
	if(m_hCaret.IsValid())
		m_hCaret->Remove();
	if(m_hText.IsValid())
	{
		auto hThis = GetHandle();
		m_hText->AddCallback("OnTextChanged",FunctionCallback<void,std::reference_wrapper<const std::string>,std::reference_wrapper<const std::string>>::Create([hThis](std::reference_wrapper<const std::string> oldText,std::reference_wrapper<const std::string> newText) {
			if(!hThis.IsValid())
				return;
			static_cast<WIKeyEntry*>(hThis.get())->OnTextChanged(oldText,newText);
		}));
	}
	ApplyKey(static_cast<GLFW::Key>(-1));
}

void WIKeyEntry::SetSize(int x,int y)
{
	WITextEntryBase::SetSize(x,y);
	if(!m_hText.IsValid())
		return;
	WIText *t = m_hText.get<WIText>();
	OnTextChanged(t->GetText(),t->GetText());
}

void WIKeyEntry::KeyboardCallback(GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods)
{
	WIBase::KeyboardCallback(key,scanCode,state,mods);
	if(state != GLFW::KeyState::Press)
		return;
	ApplyKey(key);
}
void WIKeyEntry::ScrollCallback(Vector2 offset)
{
	WIBase::ScrollCallback(offset);
	if(offset.y >= 0.f)
		ApplyKey(static_cast<GLFW::Key>(GLFW_CUSTOM_KEY_SCRL_UP));
	else
		ApplyKey(static_cast<GLFW::Key>(GLFW_CUSTOM_KEY_SCRL_DOWN));
}
void WIKeyEntry::SetKey(GLFW::Key key)
{
	if(key == m_key)
		return;
	ApplyKey(key);
}
GLFW::Key WIKeyEntry::GetKey() const {return m_key;}
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
		KeyToText(CInt16(key),&skey);
	//std::transform(skey.begin(),skey.end(),skey.begin(),::tolower);
	SetText(skey);
	m_bKeyPressed = true;
	KillFocus();
	CallCallbacks<void,GLFW::Key,GLFW::Key>("OnKeyChanged",prevKey,key);
}
void WIKeyEntry::CharCallback(unsigned int c,GLFW::Modifier mods)
{
	WIBase::CharCallback(c,mods);
}
void WIKeyEntry::OnFocusGained()
{
	WITextEntryBase::OnFocusGained();
	if(m_hMouseTrap.IsValid())
		m_hMouseTrap->Remove();
	m_previousKey = GetText();
	SetText(Locale::GetText("press_a_key"));
	auto *pRect = WGUI::GetInstance().Create<WIBase>();
	m_hMouseTrap = pRect->GetHandle();
	pRect->SetAutoAlignToParent(true);
	pRect->SetZPos(10'000);
	pRect->SetMouseInputEnabled(true);
	pRect->SetScrollInputEnabled(true);
	auto hKeyEntry = GetHandle();
	auto hRect = pRect->GetHandle();
	pRect->AddCallback("OnMouseEvent",FunctionCallback<void,GLFW::MouseButton,GLFW::KeyState,GLFW::Modifier>::Create([hRect,hKeyEntry](GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier) {
		if(state != GLFW::KeyState::Press || !hKeyEntry.IsValid())
			return;
		auto *pKeyEntry = hKeyEntry.get<WIKeyEntry>();
		pKeyEntry->ApplyKey(static_cast<GLFW::Key>(static_cast<uint32_t>(button) +static_cast<uint32_t>(GLFW::Key::Last)));
		if(hRect.IsValid())
			hRect.get()->RemoveSafely();
	}));
	pRect->AddCallback("OnScroll",FunctionCallback<void,Vector2>::Create([hRect,hKeyEntry](Vector2 offset) {
		if(!hKeyEntry.IsValid())
			return;
		auto *pKeyEntry = hKeyEntry.get<WIKeyEntry>();
		pKeyEntry->ApplyKey(static_cast<GLFW::Key>((offset.y >= 0.f) ? GLFW_CUSTOM_KEY_SCRL_UP : GLFW_CUSTOM_KEY_SCRL_DOWN));
		if(hRect.IsValid())
			hRect.get()->RemoveSafely();
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
