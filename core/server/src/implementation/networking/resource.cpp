// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :networking.resource;

pragma::networking::Resource::Resource(std::string name, bool bStream) : offset(0), stream(bStream)
{
	this->name = fs::get_canonicalized_path(name);
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
	file = pragma::fs::open_file(name.c_str(), fs::FileMode::Read | fs::FileMode::Binary);
	if(file == nullptr)
		return false;
	return true;
}
