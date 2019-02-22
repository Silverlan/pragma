#ifndef __RESOURCE_WATCHER_H__
#define __RESOURCE_WATCHER_H__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_extensible_enum.hpp>
#include <fsys/directory_watcher.h>

#define RESOURCE_WATCHER_VERBOSE 0

class DLLNETWORK EResourceWatcherCallbackType
	: public util::ExtensibleEnum
{
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
	enum class E : uint32_t
	{
		Model = 0u,
		Material,
		Texture,
		Map,
		SoundScript,
		Sound,
		Count
	};
};
DEFINE_STD_HASH_SPECIALIZATION(EResourceWatcherCallbackType);

class LuaDirectoryWatcherManager;
class DLLNETWORK ResourceWatcherManager
{
public:
	ResourceWatcherManager(NetworkState *nw);
	bool MountDirectory(const std::string &path,bool bAbsolutePath=false);
	void Poll();

	CallbackHandle AddChangeCallback(EResourceWatcherCallbackType type,const std::function<void(std::reference_wrapper<const std::string>,std::reference_wrapper<const std::string>)> &fcallback);
protected:
	NetworkState *m_networkState = nullptr;
	void OnResourceChanged(const std::string &path);
	void ReloadMaterial(const std::string &path);
	virtual void OnResourceChanged(const std::string &path,const std::string &ext);
	virtual void GetWatchPaths(std::vector<std::string> &paths);
	virtual void ReloadTexture(const std::string &path);
	void CallChangeCallbacks(EResourceWatcherCallbackType type,const std::string &path,const std::string &ext);
private:
	std::unordered_map<EResourceWatcherCallbackType,std::vector<CallbackHandle>> m_callbacks;
	std::unordered_map<std::string,std::function<void()>> m_watchFiles;
	std::vector<std::shared_ptr<DirectoryWatcherCallback>> m_watchers;
};

#endif
