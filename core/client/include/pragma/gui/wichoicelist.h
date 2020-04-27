/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WICHOICELIST_H__
#define __WICHOICELIST_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>
#include <mathutil/umath.h>

class DLLCLIENT WIChoiceList
	: public WIBase
{
public:
	struct Choice
	{
		Choice(const std::string &c,const std::string &v)
			: choice(c),value(v)
		{}
		std::string choice;
		std::string value;
	};
	WIChoiceList();
	virtual ~WIChoiceList() override;
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
	void SetChoices(const std::vector<std::string> &choices);
	void SetChoices(const std::vector<std::pair<std::string,std::string>> &choices);
	void AddChoice(const std::string &choice,const std::string &val);
	void AddChoice(const std::string &choice);
	uint32_t GetChoiceCount() const;
	void SelectChoice(UInt choice);
	void SelectChoice(const std::string &choice);
	void SelectNext();
	void SelectPrevious();
	const Choice *GetSelectedChoice();
private:
	WIHandle m_text;
	WIHandle m_buttonPrev;
	WIHandle m_buttonNext;
	std::vector<Choice> m_choices;
	UInt m_selected;
	void UpdateButtons();
};

#endif