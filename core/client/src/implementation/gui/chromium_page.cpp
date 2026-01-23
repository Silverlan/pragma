// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#ifdef PRAGMA_CHROMIUM_ENABLED
#include <../../../modules/pr_chromium/include/util_javascript.hpp>
#endif
#include <cassert>

module pragma.client;

import :gui.chromium_page;

import :client_state;

pragma::gui::types::WIChromiumPage::WIChromiumPage(WIBase *parent) : m_parent(parent) {}

void pragma::gui::types::WIChromiumPage::OnVisibilityChanged(bool bVisible)
{
	if(bVisible == true && m_bEntered == false) {
		m_bEntered = true;
		OnFirstEntered();
	}
}

void pragma::gui::types::WIChromiumPage::SetInitialURL(const std::string &url) { m_initialURL = url; }

static void (*fLoadUrl)(pragma::gui::types::WIBase *, const std::string &);
static void (*fSetViewSize)(pragma::gui::types::WIBase *, const Vector2i &);
static void (*fSetTransparentBackground)(pragma::gui::types::WIBase *, bool);
static void (*fRegisterJavascriptFunction)(const std::string &, const std::function<std::unique_ptr<pragma::JSValue>(const std::vector<pragma::JSValue> &)>);
static void (*fExecJavascript)(pragma::gui::types::WIBase *, const std::string &);

void pragma::gui::types::WIChromiumPage::LoadURL(const std::string &url)
{
	if(fLoadUrl == nullptr || m_hWeb.IsValid() == false)
		return;
	fLoadUrl(m_hWeb.get(), url);
}
void pragma::gui::types::WIChromiumPage::SetViewSize(const Vector2i &size)
{
	if(fLoadUrl == nullptr || m_hWeb.IsValid() == false)
		return;
	fSetViewSize(m_hWeb.get(), size);
}
void pragma::gui::types::WIChromiumPage::SetTransparentBackground(bool b)
{
	if(fLoadUrl == nullptr || m_hWeb.IsValid() == false)
		return;
	fSetTransparentBackground(m_hWeb.get(), b);
}
void pragma::gui::types::WIChromiumPage::RegisterJavascriptFunction(const std::string &name, const std::function<std::unique_ptr<JSValue>(const std::vector<JSValue> &)> &callback)
{
	if(fLoadUrl == nullptr)
		return;
	fRegisterJavascriptFunction(name, callback);
}
void pragma::gui::types::WIChromiumPage::ExecJavascript(const std::string &js)
{
	if(fLoadUrl == nullptr || m_hWeb.IsValid() == false)
		return;
	fExecJavascript(m_hWeb.get(), js);
}

void pragma::gui::types::WIChromiumPage::OnFirstEntered()
{
	InitializeChromium();
	if(fLoadUrl == nullptr)
		return;
	m_hWeb = WGUI::GetInstance().Create("WIWeb", m_parent)->GetHandle();
	if(m_hWeb.IsValid() == true) {
		if(fSetViewSize != nullptr)
			fSetViewSize(m_hWeb.get(), Vector2i(1024, 768));
		fSetTransparentBackground(m_hWeb.get(), true);
		m_hWeb->Update();

		InitializeWebView(m_hWeb.get());
		if(m_initialURL.empty() == false && fLoadUrl != nullptr)
			LoadURL(m_initialURL);
	}
}

void pragma::gui::types::WIChromiumPage::InitializeWebView(WIBase *el) {}

void pragma::gui::types::WIChromiumPage::InitializeJavascript() {}

void pragma::gui::types::WIChromiumPage::InitializeChromium()
{
	static auto chromiumInitialized = false;
	if(chromiumInitialized == true)
		return;
	chromiumInitialized = true;
	// Initialize chromium
	std::string err;
	const std::string chromiumPath = "chromium/wv_chromium";
	auto *client = get_client_state();
	if(client->InitializeLibrary(chromiumPath, &err) != nullptr) {
		auto dllHandle = client->GetLibraryModule(chromiumPath);
		assert(dllHandle != nullptr);
		if(dllHandle != nullptr) {
			fLoadUrl = dllHandle->FindSymbolAddress<decltype(fLoadUrl)>("wv_chromium_load_url");
			fSetViewSize = dllHandle->FindSymbolAddress<decltype(fSetViewSize)>("wv_chromium_set_browser_view_size");
			fSetTransparentBackground = dllHandle->FindSymbolAddress<decltype(fSetTransparentBackground)>("wv_chromium_set_transparent_background");
			fRegisterJavascriptFunction = dllHandle->FindSymbolAddress<decltype(fRegisterJavascriptFunction)>("wv_chromium_register_javascript_function");
			fExecJavascript = dllHandle->FindSymbolAddress<decltype(fExecJavascript)>("wv_chromium_exec_javascript");
			InitializeJavascript();
		}
	}
	else
		Con::CERR << "Unable to load 'chromium' libary: " << err << Con::endl;
}
