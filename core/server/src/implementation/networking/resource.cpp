// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.server;
import :networking.resource;

pragma::networking::Resource::Resource(std::string name, bool bStream) : offset(0), stream(bStream)
{
	this->name = FileManager::GetCanonicalizedPath(name);
	file = nullptr;
}
pragma::networking::Resource::~Resource()
{
	if(file == nullptr)
		return;
	file.reset();
}
bool pragma::networking::Resource::Construct()
{
	file = FileManager::OpenFile(name.c_str(), "rb");
	if(file == nullptr)
		return false;
	return true;
}
