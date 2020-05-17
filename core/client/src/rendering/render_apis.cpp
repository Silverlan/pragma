/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright (c) 2020 Florian Weischer
*/

#include "stdafx_client.h"
#include "pragma/rendering/render_apis.hpp"

std::vector<std::string> pragma::rendering::get_available_graphics_apis()
{
	std::vector<std::string> dirs {};
	FileManager::FindFiles(
		"modules/graphics/*",nullptr,&dirs,
		fsys::SearchFlags::Local // Binary modules can only be loaded from actual files
	);
	return dirs;
}
std::string pragma::rendering::get_graphics_api_module_location(const std::string &graphicsAPI)
{
	return "graphics/" +graphicsAPI +"/pr_prosper_" +graphicsAPI;
}
