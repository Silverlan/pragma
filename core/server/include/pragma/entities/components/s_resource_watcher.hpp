#ifndef __S_RESOURCE_WATCHER_HPP__
#define __S_RESOURCE_WATCHER_HPP__

#include "pragma/serverdefinitions.h"
#include <pragma/util/resource_watcher.h>

class DLLSERVER ESResourceWatcherCallbackType
	: public EResourceWatcherCallbackType
{
public:
	using EResourceWatcherCallbackType::EResourceWatcherCallbackType;

	static const ESResourceWatcherCallbackType NavMesh;
	static const ESResourceWatcherCallbackType Count;
protected:
	enum class E : uint32_t
	{
		NavMesh = umath::to_integral(EResourceWatcherCallbackType::E::Count),
		Count
	};
};

class DLLSERVER SResourceWatcherManager
	: public ResourceWatcherManager
{
protected:
	virtual void OnResourceChanged(const std::string &path,const std::string &ext) override;
public:
	using ResourceWatcherManager::ResourceWatcherManager;
};

#endif
