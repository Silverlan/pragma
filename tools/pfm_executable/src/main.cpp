// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "pragma/pragma_executable.hpp"

int main(int argc, char *argv[])
//try
{
	std::vector<std::string> extraArgs {
	  "-title",
	  "Pragma Filmmaker",
	  "-auto_exec",
	  "pfm/startup.lua",
	  "-icon",
	  "materials/pfm/logo/icon_window.png",
	  "-title_bar_color",
	  "#262626",
	  "+map empty",
	  "+pfm",

	  // Enabling GFX diagnostics will lower performance,
	  // but can help with debugging rendering-related crashes and
	  // for PFM rendering performance is not as critical.
	  "-enable_gfx_diagnostics",
	};
	auto cargs = pragma::merge_arguments(argc, argv, extraArgs);
	auto hModule = pragma::launch_pragma(cargs.size(), cargs.data());
	return hModule ? EXIT_SUCCESS : EXIT_FAILURE;
}
/*catch(...) {
	// Note: Calling std::current_exception in a std::set_terminate handler will return NULL due to a bug in the VS libraries.
	// Catching all unhandled exceptions here and then calling the handler works around that issue.
	std::get_terminate()();
}*/

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) { return main(__argc, __argv); }
#endif
