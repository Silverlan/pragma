/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WICHROMIUMPAGE_HPP__
#define __WICHROMIUMPAGE_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>
#include <string>
#include <mathutil/uvec.h>

class WIBase;
namespace pragma {
	class JSValue;
};
class DLLCLIENT WIChromiumPage {
  public:
	WIChromiumPage(WIBase *parent);
	void SetInitialURL(const std::string &url);
  protected:
	void InitializeChromium();
	virtual void InitializeJavascript();
	virtual void InitializeWebView(WIBase *el);
	virtual void OnVisibilityChanged(bool bVisible);
	virtual void OnFirstEntered();

	void LoadURL(const std::string &url);
	void SetViewSize(const Vector2i &size);
	void SetTransparentBackground(bool b);
	void RegisterJavascriptFunction(const std::string &name, const std::function<std::unique_ptr<pragma::JSValue>(const std::vector<pragma::JSValue> &)> &callback);
	void ExecJavascript(const std::string &js);

	bool m_bEntered = false;
	WIHandle m_hWeb;
	std::string m_initialURL;
  private:
	WIBase *m_parent = nullptr;
};

#endif
