/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __RESOURCE_WATCHER_H__
#define __RESOURCE_WATCHER_H__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_extensible_enum.hpp>
#include <sharedutils/scope_guard.h>
#include <fsys/directory_watcher.h>
#include <unordered_set>

#define RESOURCE_WATCHER_VERBOSE 0

class DLLNETWORK EResourceWatcherCallbackType : public util::ExtensibleEnum {
  public:
	using util::ExtensibleEnum::ExtensibleEnum;

	static const EResourceWatcherCallbackType Model;
	static const EResourceWatcherCallbackType Material;
	static const EResourceWatcherCallbackType Texture;
	static const EResourceWatcherCallbackType Map;
	static const EResourceWatcherCallbackType SoundScript;
	static const EResourceWatcherCallbackType Sound;
	static const EResourceWatcherCallbackType Count;
  protected:
	enum class E : uint32_t { Model = 0u, Material, Texture, Map, SoundScript, Sound, Count };
};
DEFINE_STD_HASH_SPECIALIZATION(EResourceWatcherCallbackType);

class Model;
class LuaDirectoryWatcherManager;
class DLLNETWORK ResourceWatcherManager {
  public:
	using TypeHandler = std::function<void(const util::Path &, const std::string &)>;
	ResourceWatcherManager(NetworkState *nw);
	bool MountDirectory(const std::string &path, bool bAbsolutePath = false);
	void Poll();

	// All file changes will be ignored as long as the watcher is locked.
	// It can be locked multiple times and has to be unlocked the same amount of
	// times to resume watching
	void Lock();
	void Unlock();
	util::ScopeGuard ScopeLock();
	bool IsLocked() const;
	CallbackHandle AddChangeCallback(EResourceWatcherCallbackType type, const std::function<void(std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>)> &fcallback);
	void RegisterTypeHandler(const std::string &ext, const TypeHandler &handler);
  protected:
	NetworkState *m_networkState = nullptr;
	uint32_t m_lockedCount = 0;
	std::recursive_mutex m_watcherMutex;
	void OnResourceChanged(const util::Path &rootPath, const util::Path &path);
	void ReloadMaterial(const std::string &path);
	virtual void OnMaterialReloaded(const std::string &path, const std::unordered_set<Model *> &modelMap) {}
	virtual void OnResourceChanged(const util::Path &rootPath, const util::Path &path, const std::string &ext);
	virtual void GetWatchPaths(std::vector<std::string> &paths);
	virtual void ReloadTexture(const std::string &path);
	void CallChangeCallbacks(EResourceWatcherCallbackType type, const std::string &path, const std::string &ext);
  private:
	std::unordered_map<EResourceWatcherCallbackType, std::vector<CallbackHandle>> m_callbacks;
	std::unordered_map<std::string, std::function<void()>> m_watchFiles;
	std::vector<std::shared_ptr<DirectoryWatcherCallback>> m_watchers;
	std::unordered_map<std::string, TypeHandler> m_typeHandlers;
};

#endif
