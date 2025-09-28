// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
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
