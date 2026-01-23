// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.chromium_page;

export import pragma.gui;

namespace pragma {
	class JSValue;
};
export namespace pragma::gui::types {
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
		void RegisterJavascriptFunction(const std::string &name, const std::function<std::unique_ptr<JSValue>(const std::vector<JSValue> &)> &callback);
		void ExecJavascript(const std::string &js);

		bool m_bEntered = false;
		WIHandle m_hWeb;
		std::string m_initialURL;
	  private:
		WIBase *m_parent = nullptr;
	};
};
