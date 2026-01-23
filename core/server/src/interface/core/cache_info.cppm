// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:core.cache_info;

export import std;

export struct DLLSERVER CacheInfo {
	CacheInfo() : cache(""), size(0) {}
	CacheInfo(std::string cache, unsigned int size)
	{
		this->cache = cache;
		this->size = size;
	}
	std::string cache;
	unsigned int size;
};
