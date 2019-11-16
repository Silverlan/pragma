#include "stdafx_client.h"
#include "pragma/console/c_cvar_global_functions.h"
#include <wgui/types/witext.h>
#include <wgui/types/wirect.h>

extern DLLCLIENT ClientState *client;

#pragma optimize("",off)
void Console::commands::debug_gui_cursor(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	static CallbackHandle hCbThink = {};
	static WIHandle hText = {};
	static std::array<WIHandle,4> borderElements = {};
	static std::array<WIHandle,4> borderElementsConstrained = {};
	if(hCbThink.IsValid())
	{
		hCbThink.Remove();
		if(hText.IsValid())
			hText->Remove();
		for(auto &hEl : borderElements)
		{
			if(hEl.IsValid())
				hEl->Remove();
		}
		for(auto &hEl : borderElementsConstrained)
		{
			if(hEl.IsValid())
				hEl->Remove();
		}
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
	pText->SetColor(Color::Orange);

	for(auto &hEl : borderElements)
	{
		auto *el = WGUI::GetInstance().Create<WIRect>();
		el->SetColor(Color::Red);
		el->SetZPos(std::numeric_limits<int>::max());
		hEl = el->GetHandle();
	}
	for(auto &hEl : borderElementsConstrained)
	{
		auto *el = WGUI::GetInstance().Create<WIRect>();
		el->SetColor(Color::Aqua);
		el->SetAlpha(0.75f);
		el->SetZPos(std::numeric_limits<int>::max());
		hEl = el->GetHandle();
	}

	static WIHandle hCursorElement = {};
	hCursorElement = {};
	hText = pText->GetHandle();
	hCbThink = client->AddCallback("Think",FunctionCallback<void>::Create([]() {
		if(hText.IsValid() == false)
			return;
		auto &gui = WGUI::GetInstance();
		auto *pText = static_cast<WIText*>(hText.get());
		auto *pEl = gui.GetCursorGUIElement(nullptr,[](WIBase *pEl) -> bool {
			return pEl != hText.get() && pEl->IsVisible() && 
				pEl != borderElements.at(0).get() && pEl != borderElements.at(1).get() && 
				pEl != borderElements.at(2).get() && pEl != borderElements.at(3).get() &&
				pEl != borderElementsConstrained.at(0).get() && pEl != borderElementsConstrained.at(1).get() && 
				pEl != borderElementsConstrained.at(2).get() && pEl != borderElementsConstrained.at(3).get() &&
				pEl->IsDescendantOf(hText.get()) == false;
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
					auto pos = pEl.GetAbsolutePos();
					auto size = pEl.GetSize();

					ss<<"Element: "<<std::string(typeid(pEl).name())<<
						"; Class: "<<pEl.GetClass()<<
						"; Name: "<<pEl.GetName()<<
						"; Size: ("<<size.x<<","<<size.y<<")"<<
						"; Pos: ("<<pos.x<<","<<pos.y<<")";
					auto debugInfo = pEl.GetDebugInfo();
					if(debugInfo.empty() == false)
						ss<<"; Debug Info: "<<debugInfo;
					return ss.str();
				};
				pText->SetText(fGetElementInfo(*pEl));
				Con::cout<<pText->GetText()<<Con::endl;

				// Initialize border to highlight the element
				auto pos = pEl->GetAbsolutePos();
				auto size = pEl->GetSize();
				auto constrainedEndPos = pos +size;
				auto *parent = pEl->GetParent();
				while(parent)
				{
					auto endPos = parent->GetAbsolutePos() +parent->GetSize();
					constrainedEndPos.x = umath::min(constrainedEndPos.x,endPos.x);
					constrainedEndPos.y = umath::min(constrainedEndPos.y,endPos.y);

					parent = parent->GetParent();
				}
				auto constrainedSize = constrainedEndPos -pos;
				constrainedSize.x = umath::max(constrainedSize.x,0);
				constrainedSize.y = umath::max(constrainedSize.y,0);

				auto fInitBorder = [](std::array<WIHandle,4> &elements,const Vector2i &pos,const Vector2i &size) {
					auto *top = elements.at(0).get();
					auto *right = elements.at(1).get();
					auto *bottom = elements.at(2).get();
					auto *left = elements.at(3).get();
					if(top)
					{
						top->SetPos(pos.x -1,pos.y -1);
						top->SetSize(size.x +2,1);
					}

					if(right)
					{
						right->SetPos(pos.x +size.x,pos.y -1);
						right->SetSize(1,size.y +2);
					}

					if(bottom)
					{
						bottom->SetPos(pos.x -1,pos.y +size.y);
						bottom->SetSize(size.x +2,1);
					}

					if(left)
					{
						left->SetPos(pos.x -1,pos.y -1);
						left->SetSize(1,size.y +2);
					}
				};
				fInitBorder(borderElements,pos,size);
				fInitBorder(borderElementsConstrained,pos,constrainedSize);


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
#pragma optimize("",on)
