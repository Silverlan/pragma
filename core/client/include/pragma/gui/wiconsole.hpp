/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WICONSOLE_HPP__
#define __WICONSOLE_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>
#include <queue>
#include <string_view>

class WITextEntry;
class WICommandLineEntry;
class WIFrame;
class WISnapArea;
namespace pragma::string {
	class Utf8String;
};
class DLLCLIENT WIConsole : public WIBase {
  public:
	static WIConsole *Open();
	static void Close();
	static WIConsole *GetConsole();

	enum class Mode : uint8_t { Standard = 0, SimplifiedOverlay, ExternalWindow, ExternalOwnership };

	WIConsole() = default;
	virtual void Initialize() override;
	virtual void OnRemove() override;
	virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
	virtual void RequestFocus() override;
	virtual void OnDescendantFocusGained(WIBase &el) override;
	virtual void OnDescendantFocusKilled(WIBase &el) override;

	WICommandLineEntry *GetCommandLineEntryElement();
	WITextEntry *GetTextLogElement();

	void SetExternallyOwned(bool externallyOwned);
	bool IsExternallyOwned() const;

	void SetFrame(WIFrame &frame);
	WIFrame *GetFrame();

	const pragma::string::Utf8String &GetText() const;
	void SetText(const std::string &text);
	// This will only add complete lines (lines which have
	// been ended with a new-line character). Anything
	// remaining will be returned.
	std::string_view AppendText(const std::string &text);
	void Clear();

	void SetMaxLogLineCount(uint32_t count);
	uint32_t GetMaxLogLineCount() const;
  private:
	void InitializeSnapAreas();
	void UpdateConsoleMode();
	void SetSimpleConsoleMode(bool simple, bool force = false);
	WISnapArea *CreateSnapTarget(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t xt, uint32_t yt, uint32_t wt, uint32_t ht);
	uint32_t m_maxLogLineCount = 1'000u;
	// Console output which hasn't been finished with a new-line character yet
	std::string m_pendingConsoleOutput = {};
	WIHandle m_hFrame = {};
	WIHandle m_hLog = {};
	WIHandle m_hLogBg = {};
	WIHandle m_hCommandEntry = {};
	WIHandle m_hTopSnapArea = {};
	CallbackHandle m_cbConsoleOutput = {};
	CallbackHandle m_cbMainMenuVisibility = {};
	CallbackHandle m_cbCommandEntryVisibility = {};
	Mode m_mode = Mode::Standard;

	Vector2i m_menuConsolePos = {};
	Vector2i m_menuConsoleSize = {};
};

#endif
