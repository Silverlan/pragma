#include "stdafx_client.h"
#include <wgui/types/witext.h>
#include <wgui/types/witextentry.h>
#include <wgui/types/wirect.h>
#include "pragma/gui/wiconsole.h"

WIConsole::WIConsole()
	: WIBase()
{}

WIConsole::~WIConsole()
{}

void WIConsole::OnTextEntered()
{
	WITextEntry *pTextEntry = m_hTextEntry.get<WITextEntry>();
	Print(pTextEntry->GetText());
	pTextEntry->SetText("");
	pTextEntry->RequestFocus();
}

void WIConsole::Initialize()
{
	WIBase::Initialize();
	m_hBg = CreateChild<WIRect>();
	WIRect *bg = m_hBg.get<WIRect>();
	bg->SetColor(0.2f,0.2f,0.2f,1.f);

	m_hTextEntry = CreateChild<WITextEntry>();
	WITextEntry *textEntry = m_hTextEntry.get<WITextEntry>();
	textEntry->SetPos(0,GetHeight() -textEntry->GetHeight());
	textEntry->AddCallback("OnTextEntered",FunctionCallback<>::Create(
		std::bind(&WIConsole::OnTextEntered,this)
	));

	m_hTextArea = CreateChild<WITextEntry>();
	WITextEntry *text = m_hTextArea.get<WITextEntry>();
	text->SetColor(1.f,1.f,1.f,1.f);
	text->SetKeyboardInputEnabled(false);
	text->SetMouseInputEnabled(true);
}

void WIConsole::SetSize(int x,int y)
{
	WIBase::SetSize(x,y);
	if(m_hBg.IsValid())
	{
		WIRect *bg = m_hBg.get<WIRect>();
		bg->SetSize(x,y);
	}
	if(m_hTextEntry.IsValid())
	{
		WITextEntry *textEntry = m_hTextEntry.get<WITextEntry>();
		textEntry->SetSize(GetWidth(),24);
		textEntry->SetPos(0,y -textEntry->GetHeight());

		if(m_hTextArea.IsValid())
		{
			WITextEntry *text = m_hTextArea.get<WITextEntry>();
			text->SetSize(x,y -textEntry->GetHeight());
		}
	}
}

void WIConsole::Print(const char *str)
{
	if(!m_hTextArea.IsValid())
		return;
	WITextEntry *text = m_hTextArea.get<WITextEntry>();
	std::string strText = text->GetText();
	strText += str;
	strText += "\n";
	text->SetText(strText);
}

void WIConsole::Print(const std::string &str) {Print(str.c_str());}