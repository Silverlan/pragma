// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <atomic>
#include <mutex>

export module pragma.client.gui:main_menu_mods;

import :chromium_page;
import :main_menu_base;

import pragma.client.core;

export {
	class DLLCLIENT WIMainMenuMods : public WIMainMenuBase, public WIChromiumPage {
	protected:
		WIHandle m_hWeb;
		std::shared_ptr<pragma::AddonInstallManager> m_addonInstallManager = nullptr;
		virtual void OnVisibilityChanged(bool bVisible) override;
		virtual void OnFirstEntered() override;
		virtual void InitializeJavascript() override;
		virtual void InitializeWebView(WIBase *el) override;
		bool SetAddonSubscription(const std::shared_ptr<AddonInfo> &addon, bool bSubscribe);
		bool SetAddonSubscription(const std::string &uniqueId, bool bSubscribe);

		struct DownloadInfo {
			DownloadInfo(const std::string &uniqueId);
			std::string uniqueId;
			std::shared_ptr<std::atomic<float>> downloadProgress = nullptr;
		};
		std::vector<std::unique_ptr<DownloadInfo>> m_downloads;
		std::mutex m_downloadMutex;
	public:
		WIMainMenuMods();
		virtual ~WIMainMenuMods() override;
		virtual void Initialize() override;
		virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
	};
};
