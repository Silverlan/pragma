/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__
#include "pragma/serverdefinitions.h"
#include <pragma/game/game_resources.h>
#include <vector>
#include <string>

#undef FindResource

class DLLSERVER ResourceManager
{
private:
	struct ResourceInfo
	{
		ResourceInfo(const std::string &fileName,bool stream);
		std::string fileName;
		bool stream;
	};
	static std::vector<ResourceInfo> m_resources;
public:
	static const std::vector<ResourceInfo> &GetResources();
	static bool AddResource(std::string res,bool stream=false);
	static unsigned int GetResourceCount();
	static bool IsValidResource(std::string res);
	static void ClearResources();
	static const ResourceInfo *FindResource(const std::string &fileName);
};

#endif