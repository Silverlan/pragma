#ifndef __WICHECKBOX_H__
#define __WICHECKBOX_H__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

class DLLCLIENT WICheckbox
	: public WIRect
{
protected:
	WIHandle m_hOutline;
	WIHandle m_hTick;
	bool m_bChecked;
public:
	WICheckbox();
	virtual ~WICheckbox() override;
	virtual void Initialize() override;
	void SetChecked(bool bChecked);
	bool IsChecked();
	void Toggle();
	virtual void MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) override;
	virtual void SetSize(int x,int y) override;
};

#endif