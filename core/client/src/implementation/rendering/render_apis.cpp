// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

module pragma.client;


import :rendering.render_apis;
std::vector<std::string> pragma::rendering::get_available_graphics_apis()
{
	std::vector<std::string> dirs {};
	FileManager::FindFiles("modules/graphics/*", nullptr, &dirs,
	  fsys::SearchFlags::Local // Binary modules can only be loaded from actual files
	);
	return dirs;
}
std::string pragma::rendering::get_graphics_api_module_location(const std::string &graphicsAPI)
{
#ifdef _WIN32
	return "graphics/" + graphicsAPI + "/pr_prosper_" + graphicsAPI;
#else
	return "graphics/" + graphicsAPI + "/libpr_prosper_" + graphicsAPI;
#endif
}
