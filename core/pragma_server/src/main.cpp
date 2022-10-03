/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include <string>
#include <sstream>
//#include <stdlib.h>
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

#ifdef _WIN32
static std::string get_last_system_error_string(DWORD errorMessageID)
{
    if(errorMessageID == 0)
        return "No error message has been recorded";
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,errorMessageID,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPSTR)&messageBuffer,0,nullptr);
    std::string message(messageBuffer,size);
    LocalFree(messageBuffer);
    return message;
}
#endif

static std::string GetAppPath()
{
#ifdef __linux__
	std::string path = "";
	pid_t pid = getpid();
	char buf[20] = {0};
	sprintf(buf,"%d",pid);
	std::string _link = "/proc/";
	_link.append(buf);
	_link.append("/exe");
	char proc[512];
	int ch = readlink(_link.c_str(),proc,512);
	if(ch != -1)
	{
		proc[ch] = 0;
		path = proc;
		std::string::size_type t = path.find_last_of("/");
		path = path.substr(0,t);
	}
	return path;
#else
	char path[MAX_PATH +1];
	GetModuleFileName(NULL,path,MAX_PATH +1); // Requires windows.h

	std::string appPath = path;
	appPath = appPath.substr(0,appPath.rfind("\\"));
	return appPath;
#endif
}

int main(int argc,char* argv[]) try
{
	#ifdef __linux__
		const char *library = "libshared.so";
	#else
		const char *library = "shared.dll";
	#endif
	#ifdef _WIN32
		std::string path = GetAppPath();
		path += "\\bin\\";
		path += library;
		HINSTANCE hEngine = LoadLibraryEx(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
		if(hEngine == nullptr)
		{
			auto err = GetLastError();
			std::stringstream msg;
			msg<<"Unable to load library 'bin\\"<<library<<"': "<<get_last_system_error_string(err)<<"("<<std::to_string(err)<<")";
			MessageBox(nullptr,msg.str().c_str(),"Critical Error",MB_OK | MB_ICONERROR);
			return EXIT_FAILURE;
		}
		void(*runEngine)(int,char*[]) = (void(*)(int,char*[]))GetProcAddress(hEngine,"RunEngine");
		if(runEngine != nullptr)
			runEngine(argc,argv);
	#else
		std::string path = "lib/";
		path += library;
		std::replace(path.begin(),path.end(),'\\','/');
       void *hEngine = dlopen(path.c_str(),RTLD_LAZY);
		if(hEngine == nullptr)
		{
			char *err = dlerror();
			std::cout<<"Unable to load library 'lib/"<<library<<"': "<<err<<std::endl;
			sleep(5);
			return EXIT_FAILURE;
		}
#ifdef LINUX_THREAD_TEST
        //std::thread t([]() {std::cout<<"Linux Thread Test";});
#endif
		void(*runEngine)(int,char*[]) = (void(*)(int,char*[]))dlsym(hEngine,"RunEngine");
		if(runEngine != nullptr)
			runEngine(argc,argv);
	#endif
#ifdef _DEBUG
	//_CrtDumpMemoryLeaks();
#endif
	return 0;
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
