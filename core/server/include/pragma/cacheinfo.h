/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __CACHEINFO_H__
#define __CACHEINFO_H__
#include "pragma/serverdefinitions.h"
struct DLLSERVER CacheInfo
{
	CacheInfo()
		: cache(""),size(0)
	{}
	CacheInfo(std::string cache,unsigned int size)
	{
		this->cache = cache;
		this->size = size;
	}
	std::string cache;
	unsigned int size;
};
#endif