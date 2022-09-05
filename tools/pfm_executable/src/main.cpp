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

int main(int argc,char* argv[])
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

#ifdef _WIN32
	int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
	{
		return main(__argc,__argv);
	}
#endif
