// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include <vector>
#include "pragma/pragma_executable.hpp"

int main(int argc, char *argv[])
//try
{
	std::vector<std::string> extraArgs {"-log_file", "log_server.txt", "-console_subsystem"};
	auto cargs = pragma::merge_arguments(argc, argv, extraArgs);
	auto hModule = pragma::launch_pragma(cargs.size(), cargs.data(), true);
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
