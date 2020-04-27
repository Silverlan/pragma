/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WIMESSAGEBOX_H__
#define __WIMESSAGEBOX_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class WIButton;
class DLLCLIENT WIMessageBox
	: public WIBase
{
public:
	enum class Button : Int32
	{
		NONE = 0,
		APPLY = 1,
		CANCEL = 2,
		YES = 4,
		NO = 8,
		YESNO = 12,
		ACCEPT = 16,
		DECLINE = 32,
		OK = 64
	};
private:
	static void __buttonCallback(WIHandle hMessageBox,WIMessageBox::Button button);
protected:
	WIHandle m_hBg;
	WIHandle m_hMessage;
	WIHandle m_hText;
	std::vector<WIHandle> m_buttons;
	std::function<void(WIMessageBox*,Button)> m_buttonCallback;
	WIButton *AddButton(const std::string &text,Button button);
	virtual void OnRemove() override;
public:
	WIMessageBox();
	virtual ~WIMessageBox() override;
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
	void SetTitle(const std::string &title);
	void SetText(const std::string &text);
	void EnableButtons(Button buttons);
	void SetButtonCallback(const std::function<void(WIMessageBox*,Button)> &callback);
	static void ShowMessageBox(const std::string &text,const std::string &title,Button buttons,const std::function<void(WIMessageBox*)> &onShow,const std::function<void(WIMessageBox*,Button)> &buttonCallback=nullptr);
	static WIMessageBox *Create(const std::string &text,const std::string &title="",Button buttons=Button::NONE,const std::function<void(WIMessageBox*,Button)> &callback=nullptr);
	static WIMessageBox *Create(const std::string &text,Button buttons,const std::function<void(WIMessageBox*,Button)> &callback=nullptr);
};

DLLCLIENT WIMessageBox::Button operator|(const WIMessageBox::Button &a,const WIMessageBox::Button &b);
DLLCLIENT WIMessageBox::Button operator|=(const WIMessageBox::Button &a,const WIMessageBox::Button &b);
DLLCLIENT WIMessageBox::Button operator&(const WIMessageBox::Button &a,const WIMessageBox::Button &b);

#endif