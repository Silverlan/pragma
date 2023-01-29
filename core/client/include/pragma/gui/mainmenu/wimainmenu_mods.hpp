/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WIMAINMENU_NEWGAME_HPP__
#define __WIMAINMENU_NEWGAME_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"
#include "pragma/gui/wichromiumpage.hpp"
#include <atomic>
#include <mutex>

class CurlDownloadBatch;
namespace pragma {
	class AddonInstallManager;
};
class AddonInfo;
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
	virtual void Think() override;
};

#endif
