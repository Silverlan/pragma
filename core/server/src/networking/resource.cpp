/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/networking/resource.h"
#include <fsys/filesystem.h>

Resource::Resource(std::string name, bool bStream) : offset(0), stream(bStream)
{
	this->name = FileManager::GetCanonicalizedPath(name);
	file = nullptr;
}
Resource::~Resource()
{
	if(file == nullptr)
		return;
	file.reset();
}
bool Resource::Construct()
{
	file = FileManager::OpenFile(name.c_str(), "rb");
	if(file == nullptr)
		return false;
	return true;
}
