#include "stdafx_client.h"
#include "pragma/gui/wisnaparea.hpp"
#include <wgui/types/witext.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;

LINK_WGUI_TO_CLASS(WISnapArea,WISnapArea);

WISnapArea::WISnapArea()
	: WIBase()
{}

void WISnapArea::Initialize()
{
	WIBase::Initialize();
	SetSize(50,50);
	auto *pTriggerArea = WGUI::GetInstance().Create<WIBase>(this);
	pTriggerArea->SetSize(GetSize());
	pTriggerArea->SetAnchor(0.f,0.f,1.f,1.f);
	m_hTriggerArea = pTriggerArea->GetHandle();
}

WIBase *WISnapArea::GetTriggerArea() {return m_hTriggerArea.get();}
