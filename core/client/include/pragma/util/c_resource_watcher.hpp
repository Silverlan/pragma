#ifndef __C_RESOURCE_WATCHER_HPP__
#define __C_RESOURCE_WATCHER_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/util/resource_watcher.h>

class DLLCLIENT ECResourceWatcherCallbackType
	: public EResourceWatcherCallbackType
{
public:
	using EResourceWatcherCallbackType::EResourceWatcherCallbackType;

	static const ECResourceWatcherCallbackType Shader;
	static const ECResourceWatcherCallbackType ParticleSystem;
	static const ECResourceWatcherCallbackType Count;
protected:
	enum class E : uint32_t
	{
		Shader = umath::to_integral(EResourceWatcherCallbackType::E::Count),
		ParticleSystem,
		Count
	};
};

class DLLCLIENT CResourceWatcherManager
	: public ResourceWatcherManager
{
protected:
	virtual void OnResourceChanged(const std::string &path,const std::string &ext) override;
	virtual void ReloadTexture(const std::string &path) override;
	virtual void GetWatchPaths(std::vector<std::string> &paths) override;
public:
	using ResourceWatcherManager::ResourceWatcherManager;
};

#endif
