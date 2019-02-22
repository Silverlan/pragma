#include "stdafx_client.h"
#include "pragma/console/c_cvar_global_functions.h"
#include <wgui/types/witext.h>

extern DLLCLIENT ClientState *client;

void Console::commands::debug_gui_cursor(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	static CallbackHandle hCbThink = {};
	static WIHandle hText = {};
	if(hCbThink.IsValid())
	{
		hCbThink.Remove();
		if(hText.IsValid())
			hText->Remove();
		return;
	}
	auto &gui = WGUI::GetInstance();
	auto *pText = gui.Create<WIText>();
	if(pText == nullptr)
		return;
	if(hText.IsValid())
		hText->Remove();
	pText->SetZPos(std::numeric_limits<int32_t>::max());
	pText->SetVisible(true);
	pText->EnableShadow(true);
	pText->SetShadowOffset(Vector2i(1,1));
	pText->SetColor(Color::White);
	static WIHandle hCursorElement = {};
	hCursorElement = {};
	hText = pText->GetHandle();
	auto cb = client->AddCallback("Think",FunctionCallback<void>::Create([]() {
		if(hText.IsValid() == false)
			return;
		auto &gui = WGUI::GetInstance();
		auto *pText = static_cast<WIText*>(hText.get());
		auto *pEl = gui.GetCursorGUIElement(nullptr,[](WIBase *pEl) -> bool {
			return pEl != hText.get() && pEl->IsDescendantOf(hText.get()) == false;
		});
		if(pEl != hCursorElement.get())
		{
			hCursorElement = (pEl != nullptr) ? pEl->GetHandle() : WIHandle{};
			if(pEl == nullptr)
			{
				pText->SetText("No element.");
				Con::cout<<pText->GetText()<<Con::endl;
			}
			else
			{
				auto fGetElementInfo = [](WIBase &pEl) -> std::string {
					std::stringstream ss;
					ss<<"Element: "<<std::string(typeid(pEl).name())<<"; Class: "<<pEl.GetClass()<<"; Name: "<<pEl.GetName();
					auto debugInfo = pEl.GetDebugInfo();
					if(debugInfo.empty() == false)
						ss<<"; Debug Info: "<<debugInfo;
					return ss.str();
				};
				pText->SetText(fGetElementInfo(*pEl));
				Con::cout<<pText->GetText()<<Con::endl;

				std::string t = "\t";
				auto *pParent = pEl->GetParent();
				while(pParent != nullptr)
				{
					Con::cout<<t<<fGetElementInfo(*pParent)<<Con::endl;
					pParent = pParent->GetParent();
					t += "\t";
				}
			}
			pText->SizeToContents();
			
		}
		int32_t x,y;
		gui.GetMousePos(x,y);
		hText.get()->SetPos(x,y);
	}));
}

