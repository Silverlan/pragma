#ifndef __WIMAINMENU_BASE_H__
#define __WIMAINMENU_BASE_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>
#include <sharedutils/functioncallback.h>

class WIMainMenuElement;
class WIOptionsList;
class DLLCLIENT WIMainMenuBase
	: public WIBase
{
protected:
	std::vector<WIHandle> m_elements;
	int m_selected;
	WIHandle m_hControlSettings;
	void OnGoBack(int button,int action,int mods);
	virtual void InitializeOptionsList(WIOptionsList *pList);
	WIOptionsList *InitializeOptionsList();
public:
	WIMainMenuBase();
	virtual void Initialize() override;
	virtual void MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) override;
	virtual void KeyboardCallback(GLFW::Key key,int scanCode,GLFW::KeyState state,GLFW::Modifier mods) override;
	void SelectItem(int i);
	void SelectNextItem();
	void SelectPreviousItem();
	WIMainMenuElement *GetElement(int i);
	WIMainMenuElement *GetSelectedElement();
	void AddMenuItem(std::string name,const CallbackHandle &onActivated);
	void AddMenuItem(int pos,std::string name,const CallbackHandle &onActivated);
	void RemoveMenuItem(int i);
	void OnElementSelected(WIMainMenuElement *el);
	void UpdateElements();
	void UpdateElement(int i);
};

class DLLCLIENT WIMainMenuElement
	: public WIBase
{
protected:
	WIHandle m_hBackground;
	WIHandle m_hText;
	bool m_bSelected;
public:
	WIMainMenuElement();
	virtual ~WIMainMenuElement() override;
	virtual void Initialize() override;
	void Select();
	void Deselect();
	void SetText(std::string &text);
	virtual void MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) override;
	virtual void OnCursorEntered() override;
	virtual void OnCursorExited() override;
	void Activate();
	virtual void SetSize(int x,int y) override;
	Vector4 GetBackgroundColor();
	void SetBackgroundColor(float r,float g,float b,float a=1.f);

	CallbackHandle onActivated;
	CallbackHandle onSelected;
	CallbackHandle onDeselected;
};

#endif