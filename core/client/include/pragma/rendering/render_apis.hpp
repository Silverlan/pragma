/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright (c) 2020 Florian Weischer
*/

#ifndef __RENDER_APIS_HPP__
#define __RENDER_APIS_HPP__

#include "pragma/clientdefinitions.h"
#include <string>
#include <vector>

namespace pragma::rendering
{
	constexpr std::string_view RENDERING_API_MODULE_LOCATION = "graphics";
	DLLCLIENT std::vector<std::string> get_available_graphics_apis();
	DLLCLIENT std::string get_graphics_api_module_location(const std::string &graphicsAPI);
};

#endif