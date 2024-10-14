/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ADDONSYSTEM_H__
#define __ADDONSYSTEM_H__

#include "pragma/networkdefinitions.h"
#include <vector>
#include <string>

#define FSYS_SEARCH_ADDON 2048

class AddonSystem;
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLNETWORK AddonInfo {
  public:
	friend AddonSystem;
	AddonInfo(const std::string &absPath, const util::Version &version = {}, const std::string &uniqueId = "");
  protected:
	std::string m_path;
	std::string m_uniqueId;
	util::Version m_version;
  public:
	// Note: The addon directory can be a shortcut, in which case it will have the
	// .lnk-extension on Windows systems and the path will have to be resolved to
	// get the actual addon path!
	const std::string &GetLocalPath() const;
	std::string GetAbsolutePath() const;
	const std::string &GetUniqueId() const;
	const util::Version &GetVersion() const;
};

class DirectoryWatcherCallback;
namespace pragma::pad {
	class PADPackage;
};
class DLLNETWORK AddonSystem {
  public:
	static bool MountAddon(const std::string &addonPath);
	static void MountAddons();
	static void UnmountAddons();
	static const std::vector<AddonInfo> &GetMountedAddons();
	static void Poll();
	static DirectoryWatcherCallback *GetAddonWatcher();
  private:
	static bool MountAddon(const std::string &addonPath, std::vector<AddonInfo> &outAddons, bool silent = true);
	static std::vector<AddonInfo> m_addons;
	static std::shared_ptr<DirectoryWatcherCallback> m_addonWatcher;
	static pragma::pad::PADPackage *LoadPADPackage(const std::string &path);
};
#pragma warning(pop)

#endif
