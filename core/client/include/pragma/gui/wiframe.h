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

class DLLCLIENT WIFrame : public WITransformable {
  public:
	WIFrame();
	virtual ~WIFrame() override;
	virtual void Initialize() override;
	virtual void SetSize(int x, int y) override;
	WIBase *GetContents();
	using WITransformable::SetSize;
	void SetTitle(std::string title);
	const pragma::string::Utf8String &GetTitle() const;
	void SetCloseButtonEnabled(bool b);
	void SetDetachButtonEnabled(bool b);
	virtual util::EventReply MouseCallback(GLFW::MouseButton button, GLFW::KeyState state, GLFW::Modifier mods) override;

	void Detach();
	void Reattach();
	bool IsDetached() const;
  protected:
	WIHandle m_hBg;
	WIHandle m_hTitle;
	WIHandle m_hTitleBar;
	WIHandle m_hClose;
	WIHandle m_hDetachButton;
	WIHandle m_hContents;
	void OnDetachButtonPressed();
};

#endif
