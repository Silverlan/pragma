// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __RENDER_APIS_HPP__
#define __RENDER_APIS_HPP__

#include "pragma/clientdefinitions.h"
#include <string>
#include <vector>

namespace pragma::rendering {
	constexpr std::string_view RENDERING_API_MODULE_LOCATION = "graphics";
	DLLCLIENT std::vector<std::string> get_available_graphics_apis();
	DLLCLIENT std::string get_graphics_api_module_location(const std::string &graphicsAPI);
};

#endif
