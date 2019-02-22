#include "stdafx_client.h"
#include "pragma/gui/wislider.h"
#include <wgui/types/witext.h>
#include <wgui/types/wibutton.h>
#include "pragma/gui/wiprogressbar.h"

LINK_WGUI_TO_CLASS(WISlider,WISlider);

WISlider::WISlider()
	: WIProgressBar(),m_bMoveSlider(false)
{
	SetMouseInputEnabled(true);
}

WISlider::~WISlider()
{}

void WISlider::Initialize()
{
	WIProgressBar::Initialize();
}

void WISlider::SetSize(int x,int y)
{
	WIProgressBar::SetSize(x,y);
}

void WISlider::Think()
{
	WIProgressBar::Think();
	if(m_bMoveSlider == false)
		return;
	int x;
	GetMousePos(&x,nullptr);
	auto width = GetWidth();
	float v = CFloat(x) /CFloat(width);
	SetProgress(v);
}

void WISlider::MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods)
{
	WIProgressBar::MouseCallback(button,state,mods);
	if(button == GLFW::MouseButton::Left)
	{
		if(state == GLFW::KeyState::Press)
			m_bMoveSlider = true;
		else
			m_bMoveSlider = false;
	}
}