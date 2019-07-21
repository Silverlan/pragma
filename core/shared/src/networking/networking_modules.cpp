#include "pragma/networking/networking_modules.hpp"
#include <fsys/filesystem.h>

std::vector<std::string> pragma::networking::GetAvailableNetworkingModules()
{
	std::vector<std::string> dirs {};
	FileManager::FindFiles(
		"modules/networking/*",nullptr,&dirs,
		fsys::SearchFlags::Local // Binary modules can only be loaded from actual files
	);
	return dirs;
}
std::string pragma::networking::GetNetworkingModuleLocation(const std::string &netModule,bool server)
{
	return "networking/" +netModule +"/pr_" +netModule +(server ? "_server" : "_client");
}
