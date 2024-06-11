/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_EXECUTABLE_HPP__
#define __PRAGMA_EXECUTABLE_HPP__

#include <string>
#include <sstream>
#include <vector>

// If gDEbugger Support is enabled, all .dll-files will have to be copied to the .exe directory!
#define ENABLE_GDEBUGGER_SUPPORT 0

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#ifdef _WIN32
#include <windows.h>
#else
#define LINUX_THREAD_TEST
#ifdef LINUX_THREAD_TEST
#include <unistd.h>
#include <dlfcn.h>
#include <algorithm>
#include <iostream>
#include <thread>
#endif
#endif

static std::string GetAppPath()
{
#ifdef __linux__
	std::string path = "";
	pid_t pid = getpid();
	char buf[20] = {0};
	sprintf(buf, "%d", pid);
	std::string _link = "/proc/";
	_link.append(buf);
	_link.append("/exe");
	char proc[512];
	int ch = readlink(_link.c_str(), proc, 512);
	if(ch != -1) {
		proc[ch] = 0;
		path = proc;
		std::string::size_type t = path.find_last_of("/");
		path = path.substr(0, t);
	}
	return path;
#else
	char path[MAX_PATH + 1];
	GetModuleFileName(NULL, path, MAX_PATH + 1); // Requires windows.h

	std::string appPath = path;
	appPath = appPath.substr(0, appPath.rfind("\\"));
	return appPath;
#endif
}

#ifdef _WIN32
static std::string get_last_system_error_string(DWORD errorMessageID)
{
	if(errorMessageID == 0)
		return "No error message has been recorded";
	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);
	std::string message(messageBuffer, size);
	LocalFree(messageBuffer);
	return message;
}
#endif

#ifdef _WIN32
#define MODULE_HANDLE HINSTANCE
#else
#define MODULE_HANDLE void *
#endif
#define MODULE_NULL                                                                                                                                                                                                                                                                              \
	std::unique_ptr<ModuleWrapper> {}

namespace pragma {
	struct ModuleWrapper {
		static std::unique_ptr<ModuleWrapper> Create(MODULE_HANDLE handle) { return std::unique_ptr<ModuleWrapper> {new ModuleWrapper {handle}}; }
		~ModuleWrapper()
		{
#ifdef _WIN32
			FreeLibrary(handle);
#else
			dlclose(handle);
#endif
		}
		MODULE_HANDLE GetHandle() { return handle; }
	  private:
		ModuleWrapper(MODULE_HANDLE &handle) : handle {handle} {}
		MODULE_HANDLE handle;
	};

	static std::unique_ptr<ModuleWrapper> launch_pragma(int argc, char *argv[], bool server = false)
	{
#ifdef __linux__
        const char *library = server ? "libshared.so" : "libclient.so";
#else
        const char *library = server ? "shared.dll" : "client.dll";
#endif
		const char *runEngineSymbol = server ? "RunEngine" : "RunCEngine";

#ifdef _WIN32
#if ENABLE_GDEBUGGER_SUPPORT == 1
		HINSTANCE hEngine = LoadLibrary(library);
#else
		std::string path = GetAppPath();
		path += "\\bin\\";
		path += library;
		HINSTANCE hEngine = LoadLibraryEx(path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
#endif
		if(hEngine == nullptr) {
			auto err = GetLastError();
			std::stringstream msg;
			msg << "Unable to load library 'bin\\" << library << "': " << get_last_system_error_string(err) << "(" << std::to_string(err) << ")";
			MessageBox(nullptr, msg.str().c_str(), "Critical Error", MB_OK | MB_ICONERROR);
			return MODULE_NULL;
		}

		auto wrapper = ModuleWrapper::Create(hEngine);
		void (*runEngine)(int, char *[]) = (void (*)(int, char *[]))GetProcAddress(hEngine, runEngineSymbol);
		if(runEngine != nullptr) {
			runEngine(argc, argv);
			return wrapper;
		}
		wrapper = {};
		return MODULE_NULL;
#else
		std::string path = "lib/";
		path += library;
		std::replace(path.begin(), path.end(), '\\', '/');
		void *hEngine = dlopen(path.c_str(), RTLD_LAZY);
		if(hEngine == nullptr) {
			char *err = dlerror();
			std::cout << "Unable to load library 'lib/" << library << "': " << err << std::endl;
			sleep(5);
			return MODULE_NULL;
		}
		auto wrapper = ModuleWrapper::Create(hEngine);
#if 0
		std::thread t([]() { std::cout << "Linux Thread Test"; });
#endif
        void (*runEngine)(int, char *[]) = (void (*)(int, char *[]))dlsym(hEngine, runEngineSymbol);
		if(runEngine != nullptr) {
			runEngine(argc, argv);
			return wrapper;
		}
		wrapper = {};
		return MODULE_NULL;
#endif
#ifdef _DEBUG
		//_CrtDumpMemoryLeaks();
#endif
	}

	static std::vector<char *> merge_arguments(int argc, char *argv[], std::vector<std::string> &args)
	{
		auto tmpArgs = std::move(args);
		args.clear();

		args.reserve(tmpArgs.size() + argc);
		for(auto i = decltype(argc) {0u}; i < argc; ++i)
			args.push_back(argv[i]);

		for(auto &arg : tmpArgs)
			args.push_back(arg);

		std::vector<char *> cargs;
		cargs.reserve(args.size());
		for(auto &arg : args)
			cargs.push_back(arg.data());
		return cargs;
	}
};

#endif
