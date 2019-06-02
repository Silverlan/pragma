#ifndef __WISLIDER_H__
#define __WISLIDER_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>
#include "pragma/gui/wiprogressbar.h"
#include <mathutil/umath.h>

class DLLCLIENT WISlider
	: public WIProgressBar
{
private:
	bool m_bMoveSlider;
public:
	WISlider();
	virtual ~WISlider() override;
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
	virtual util::EventReply MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) override;
	virtual void Think() override;

	bool IsBeingDragged() const;
};

#endif