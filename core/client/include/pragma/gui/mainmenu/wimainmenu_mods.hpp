// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
	virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
};

#endif
