/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WIKEYENTRY_H__
#define __WIKEYENTRY_H__

#include "pragma/clientdefinitions.h"
#include <wgui/types/witextentry.h>
#include <unordered_map>

class DLLCLIENT WIKeyEntry : public WITextEntryBase {
  protected:
	std::string m_previousKey;
	pragma::platform::Key m_key;
	bool m_bKeyPressed;
	WIHandle m_hMouseTrap;
	virtual void OnTextChanged(const pragma::string::Utf8String &text, bool changedByUser) override;
	void ApplyKey(pragma::platform::Key key);
  public:
	WIKeyEntry();
	virtual ~WIKeyEntry() override;
	virtual void Initialize() override;
	virtual void SetSize(int x, int y) override;
	virtual util::EventReply KeyboardCallback(pragma::platform::Key key, int scanCode, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;
	virtual util::EventReply ScrollCallback(Vector2 offset, bool offsetAsPixels = false) override;
	virtual util::EventReply CharCallback(unsigned int c, pragma::platform::Modifier mods = pragma::platform::Modifier::None) override;
	virtual void OnFocusGained() override;
	virtual void OnFocusKilled() override;
	void SetKey(pragma::platform::Key key);
	pragma::platform::Key GetKey() const;
};

#endif
