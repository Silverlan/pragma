/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
	using WITransformable::SetSize;
	void SetTitle(std::string title);
	std::string GetTitle() const;
	void SetCloseButtonEnabled(bool b);
	virtual util::EventReply MouseCallback(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods) override;
};

#endif