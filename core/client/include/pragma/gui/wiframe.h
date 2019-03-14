#ifndef __WIFRAME_H__
#define __WIFRAME_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/witransformable.h"

class DLLCLIENT WIFrame
	: public WITransformable
{
protected:
	WIHandle m_hBg;
	WIHandle m_hTitle;
	WIHandle m_hTitleBar;
	WIHandle m_hClose;
public:
	WIFrame();
	virtual ~WIFrame() override;
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
	void SetTitle(std::string title);
	std::string GetTitle() const;
	void SetCloseButtonEnabled(bool b);
	virtual void MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) override;
};

#endif