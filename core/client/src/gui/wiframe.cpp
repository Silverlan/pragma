#include "stdafx_client.h"
#include "pragma/gui/wiframe.h"
#include <wgui/types/witext.h>
#include <wgui/types/wibutton.h>
#include <wgui/types/wirect.h>
#include <mathutil/umath.h>

LINK_WGUI_TO_CLASS(WIFrame,WIFrame);

#define WIFRAME_DRAG_OFFSET_BORDER 5
#define WIFRAME_RESIZE_OFFSET_BORDER 5

WIFrame::WIFrame()
	: WITransformable()
{}
WIFrame::~WIFrame()
{
	if(m_resizeMode != ResizeMode::none)
		WGUI::GetInstance().SetCursor(GLFW::Cursor::Shape::Arrow);
}

void WIFrame::Initialize()
{
	SetDraggable(true);
	SetResizable(true);

	m_hBg = CreateChild<WIRect>();
	m_hBg->SetAutoAlignToParent(true);
	m_hBg->GetColorProperty()->Link(*GetColorProperty());
	WITransformable::Initialize();
	if(m_hMoveRect.IsValid())
	{
		auto &gui = WGUI::GetInstance();
		m_hTitleBar = gui.Create<WIRect>(m_hMoveRect.get())->GetHandle();
		WIRect *pTitleBar = m_hTitleBar.get<WIRect>();
		pTitleBar->AddStyleClass("frame_titlebar");
		pTitleBar->SetAutoAlignToParent(true);
		auto hFrame = GetHandle();
		pTitleBar->AddCallback("SetSize",FunctionCallback<>::Create([hFrame]() {
			if(!hFrame.IsValid())
				return;
			auto *pFrame = hFrame.get<WIFrame>();
			if(!pFrame->m_hTitleBar.IsValid())
				return;
			auto *pTitleBar = pFrame->m_hTitleBar.get<WIRect>();
			if(pFrame->m_hClose.IsValid())
			{
				WIButton *pButton = pFrame->m_hClose.get<WIButton>();
				pButton->SetX(pTitleBar->GetWidth() -pButton->GetWidth() -10);
				pButton->SetY(CInt32(pTitleBar->GetHeight() *0.5f -pButton->GetHeight() *0.5f));
			}
			if(pFrame->m_hTitle.IsValid())
			{
				WIText *pText = pFrame->m_hTitle.get<WIText>();
				pText->SetX(10);
				pText->SetY(CInt32(pTitleBar->GetHeight() *0.5f -pText->GetHeight() *0.5f));
			}
		}));
	
		m_hTitle = gui.Create<WIText>(pTitleBar)->GetHandle();
		WIText *pTitle = m_hTitle.get<WIText>();
		pTitle->AddStyleClass("frame_title");
		if(pTitle != nullptr)
			pTitle->SetVisible(false);

		m_hClose = gui.Create<WIButton>(pTitleBar)->GetHandle();
		WIButton *pButton = m_hClose.get<WIButton>();
		pButton->SetText("X");
		pButton->AddCallback("OnPressed",FunctionCallback<>::Create(std::bind(&WIFrame::OnCloseButtonPressed,this)));
		pButton->SetSize(20,20);
	}
}
void WIFrame::MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods)
{
	WITransformable::MouseCallback(button,state,mods);
	RequestFocus();
}
void WIFrame::SetCloseButtonEnabled(bool b)
{
	if(!m_hClose.IsValid())
		return;
	m_hClose->SetVisible(b);
}
void WIFrame::SetSize(int x,int y)
{
	WITransformable::SetSize(x,y);
	if(m_hMoveRect.IsValid())
		m_hMoveRect->SetWidth(x);
	Vector2i minSize = m_minSize;
	if(x < minSize.x || y < minSize.y)
	{
		if(x < minSize.x)
			minSize.x = x;
		if(y < minSize.y)
			minSize.y = y;
		SetMinSize(minSize);
	}
	Vector2i maxSize = m_maxSize;
	if((x > maxSize.x && maxSize.x != -1) || (y > maxSize.y && maxSize.y != -1))
	{
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
	WIText *pText = m_hTitle.get<WIText>();
	pText->SetText(title);
	pText->SizeToContents();
	pText->SetVisible(!title.empty());
}
std::string WIFrame::GetTitle() const
{
	if(!m_hTitle.IsValid())
		return "";
	return m_hTitle.get<WIText>()->GetText();
}