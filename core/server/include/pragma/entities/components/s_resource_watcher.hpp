/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_RESOURCE_WATCHER_HPP__
#define __S_RESOURCE_WATCHER_HPP__

#include "pragma/serverdefinitions.h"
#include <pragma/util/resource_watcher.h>

class DLLSERVER ESResourceWatcherCallbackType : public EResourceWatcherCallbackType {
  public:
	using EResourceWatcherCallbackType::EResourceWatcherCallbackType;

	static const ESResourceWatcherCallbackType NavMesh;
	static const ESResourceWatcherCallbackType Count;
  protected:
	enum class E : uint32_t { NavMesh = umath::to_integral(EResourceWatcherCallbackType::E::Count), Count };
};

class DLLSERVER SResourceWatcherManager : public ResourceWatcherManager {
  protected:
	virtual void OnResourceChanged(const util::Path &rootPath, const util::Path &path, const std::string &ext) override;
  public:
	using ResourceWatcherManager::ResourceWatcherManager;
};

#endif
