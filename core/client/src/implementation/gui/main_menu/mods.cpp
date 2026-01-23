// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#ifdef PRAGMA_CHROMIUM_ENABLED
#include <../../../modules/pr_chromium/include/util_javascript.hpp>
#endif

#define PAD_ADDON_VERBOSE 1

module pragma.client;

import :gui.main_menu_mods;
import :gui.progress_bar;

import :client_state;
import :core;
import :engine;

/////////////////////////

pragma::gui::types::WIMainMenuMods::DownloadInfo::DownloadInfo(const std::string &_uniqueId) : uniqueId(_uniqueId), downloadProgress(pragma::util::make_shared<std::atomic<float>>(0.f)) {}

/////////////////////////

pragma::gui::types::WIMainMenuMods::WIMainMenuMods() : WIMainMenuBase(), WIChromiumPage(this) {}

pragma::gui::types::WIMainMenuMods::~WIMainMenuMods()
{
	if(m_addonInstallManager != nullptr) {
		m_addonInstallManager->CancelDownload();
		m_addonInstallManager = nullptr;
	}
}

void pragma::gui::types::WIMainMenuMods::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	WIBase::Think(drawCmd);
	if(IsVisible() == true) {
		m_downloadMutex.lock();
		for(auto it = m_downloads.begin(); it != m_downloads.end();) {
			auto &dlInfo = *it;
			ExecJavascript("set_addon_download_progress('" + dlInfo->uniqueId + "'," + std::to_string(*dlInfo->downloadProgress) + ");");
			if(*dlInfo->downloadProgress == 1.f)
				it = m_downloads.erase(it);
			else
				++it;
		}
		m_downloadMutex.unlock();
	}
}

bool pragma::gui::types::WIMainMenuMods::SetAddonSubscription(const std::shared_ptr<AddonInfo> &addon, bool bSubscribe)
{
	m_downloadMutex.lock();
	auto &uniqueId = addon->GetUniqueId();
	if(bSubscribe == true) {
		if(m_addonInstallManager == nullptr) {
			m_downloadMutex.unlock();
			return false;
		}
		m_downloads.push_back(std::unique_ptr<DownloadInfo>(new DownloadInfo(uniqueId)));
		auto &dl = m_downloads.back();
		m_addonInstallManager->CheckForUpdates(addon, dl->downloadProgress);
	}
	else {
		auto it = std::find_if(m_downloads.begin(), m_downloads.end(), [&uniqueId](const std::unique_ptr<DownloadInfo> &dlInfo) { return (dlInfo->uniqueId == uniqueId) ? true : false; });
		if(it != m_downloads.end())
			m_downloads.erase(it);
		m_addonInstallManager->CancelDownload();

		// Remove temporary update file if it exists
		fs::remove_file(("addons\\" + uniqueId + ".pad.update"));
	}
	m_downloadMutex.unlock();
	return true;
}

bool pragma::gui::types::WIMainMenuMods::SetAddonSubscription(const std::string &uniqueId, bool bSubscribe) { return SetAddonSubscription(pragma::util::make_shared<AddonInfo>("", util::Version(0, 0, 0), uniqueId), bSubscribe); }

void pragma::gui::types::WIMainMenuMods::OnFirstEntered()
{
	auto lan = locale::get_language();
	if(lan != "de")
		lan = "en";
	// SetInitialURL(engine_info::get_modding_hub_url() + "index_game.php?lan=" + lan);

	try {
		auto &addons = AddonSystem::GetMountedAddons();
		m_addonInstallManager = pragma::util::make_shared<AddonInstallManager>();

		for(auto &addon : addons) {
			auto &uniqueId = addon.GetUniqueId();
			if(uniqueId.empty() == true)
				continue;
			SetAddonSubscription(pragma::util::make_shared<AddonInfo>(addon), true);
		}
	}
	catch(const std::runtime_error &e) {
	}
	WIChromiumPage::OnFirstEntered();
}

void pragma::gui::types::WIMainMenuMods::InitializeWebView(WIBase *el)
{
	WIChromiumPage::InitializeWebView(el);
	el->SetSize(1024, 768);
	el->SetPos(100, 200);
}

void pragma::gui::types::WIMainMenuMods::OnVisibilityChanged(bool bVisible)
{
	WIBase::OnVisibilityChanged(bVisible);
	WIChromiumPage::OnVisibilityChanged(bVisible);
}

void pragma::gui::types::WIMainMenuMods::InitializeJavascript()
{
	WIChromiumPage::InitializeJavascript();
#ifdef PRAGMA_CHROMIUM_ENABLED
	auto hThis = GetHandle();
	RegisterJavascriptFunction("set_addon_subscription", [hThis](const std::vector<pragma::JSValue> &args) -> std::unique_ptr<pragma::JSValue> {
		auto r = std::make_unique<pragma::JSValue>();
		r->type = pragma::JSValueType::Bool;
		r->data = pragma::util::make_shared<bool>(false);
		if(hThis.IsValid() == false || args.size() < 2 || args.at(0).type != pragma::JSValueType::String || args.at(1).type != pragma::JSValueType::Bool)
			return r;
		auto &uniqueId = *static_cast<std::string *>(args.at(0).data.get());
		if(static_cast<WIMainMenuMods *>(hThis.get())->SetAddonSubscription(uniqueId, *static_cast<bool *>(args.at(1).data.get())) == true)
			*static_cast<bool *>(r->data.get()) = true;
		return r;
	});
	RegisterJavascriptFunction("is_subscribed_to_addon", [](const std::vector<pragma::JSValue> &args) -> std::unique_ptr<pragma::JSValue> {
		auto r = std::make_unique<pragma::JSValue>();
		r->type = pragma::JSValueType::Bool;
		r->data = pragma::util::make_shared<bool>(false);
		if(args.empty() == true || args.front().type != pragma::JSValueType::String)
			return r;
		auto &uniqueId = *static_cast<std::string *>(args.at(0).data.get());
		auto &addons = AddonSystem::GetMountedAddons();
		auto it = std::find_if(addons.begin(), addons.end(), [&uniqueId](const AddonInfo &addon) { return (addon.GetUniqueId() == uniqueId) ? true : false; });
		if(it != addons.end())
			*static_cast<bool *>(r->data.get()) = true;
		return r;
	});
#endif
}

void pragma::gui::types::WIMainMenuMods::Initialize()
{
	WIMainMenuBase::Initialize();
	EnableThinking();
	AddMenuItem(locale::get_text("back"), FunctionCallback<void, WIMainMenuElement *>::Create([this](WIMainMenuElement *) {
		auto *mainMenu = dynamic_cast<WIMainMenu *>(GetParent());
		if(mainMenu == nullptr)
			return;
		mainMenu->OpenMainMenu();
	}));
}
