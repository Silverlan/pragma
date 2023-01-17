/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/pragma_executable.hpp"
#include <vector>
#include <array>
#include <string>

int main(int argc,char* argv[]) try
{
	std::vector<std::string> extraArgs {
		"-title","Pragma Filmmaker",
		"-auto_exec","pfm/startup.lua",
		"-icon","materials/pfm/logo/icon_window.png",
		"+map empty",
		"+pfm"
	};
	auto cargs = pragma::merge_arguments(argc,argv,extraArgs);
	auto hModule = pragma::launch_pragma(cargs.size(),cargs.data());
	return hModule ? EXIT_SUCCESS : EXIT_FAILURE;
}
catch(const std::exception& e) //it would not work if you pass by value
{
	// Note: Calling std::current_exception in a std::set_terminate handler will return NULL due to a bug in the VS libraries.
	// Catching all unhandled exceptions here and then calling the handler works around that issue.

    std::get_terminate();

}

#ifdef _WIN32
	int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
	{
		return main(__argc,__argv);
	}
#endif
