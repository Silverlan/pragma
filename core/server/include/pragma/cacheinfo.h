// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __CACHEINFO_H__
#define __CACHEINFO_H__
#include "pragma/serverdefinitions.h"
struct DLLSERVER CacheInfo {
	CacheInfo() : cache(""), size(0) {}
	CacheInfo(std::string cache, unsigned int size)
	{
		this->cache = cache;
		this->size = size;
	}
	std::string cache;
	unsigned int size;
};
#endif
