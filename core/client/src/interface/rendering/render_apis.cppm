// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.render_apis;

export import std;

export namespace pragma::rendering {
	constexpr std::string_view RENDERING_API_MODULE_LOCATION = "graphics";
	DLLCLIENT std::vector<std::string> get_available_graphics_apis();
	DLLCLIENT std::string get_graphics_api_module_location(const std::string &graphicsAPI);
};
