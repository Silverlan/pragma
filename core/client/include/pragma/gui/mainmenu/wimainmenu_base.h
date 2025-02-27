/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WIMAINMENU_BASE_H__
#define __WIMAINMENU_BASE_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>
#include <sharedutils/functioncallback.h>

class WIMainMenuElement;
class WIOptionsList;
class DLLCLIENT WIMainMenuBase : public WIBase {
  public:
	WIMainMenuBase();
	virtual void Initialize() override;
	virtual util::EventReply MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;
	virtual util::EventReply KeyboardCallback(pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;
	void SelectItem(int i);
	void SelectNextItem();
	void SelectPreviousItem();
	WIMainMenuElement *GetElement(int i);
	WIMainMenuElement *GetSelectedElement();
	void AddMenuItem(std::string name, const CallbackHandle &onActivated);
	void AddMenuItem(int pos, std::string name, const CallbackHandle &onActivated);
	void RemoveMenuItem(int i);
	void OnElementSelected(WIMainMenuElement *el);
	void UpdateElements();
	void UpdateElement(int i);
  protected:
	std::vector<WIHandle> m_elements;
	std::vector<WIHandle> m_optionLists = {};
	WIHandle m_menuElementsContainer = {};
	int m_selected;
	WIHandle m_hControlSettings;
	void OnGoBack(int button, int action, int mods);
	virtual void InitializeOptionsList(WIOptionsList *pList);
	WIOptionsList *InitializeOptionsList();
	virtual void DoUpdate() override;
};

class DLLCLIENT WIMainMenuElement : public WIBase {
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
	virtual util::EventReply MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;
	virtual void OnCursorEntered() override;
	virtual void OnCursorExited() override;
	void Activate();
	virtual void SetSize(int x, int y) override;
	Vector4 GetBackgroundColor();
	void SetBackgroundColor(float r, float g, float b, float a = 1.f);

	CallbackHandle onActivated;
	CallbackHandle onSelected;
	CallbackHandle onDeselected;
};

#endif
