#ifndef __SERVERDEFINITIONS_H__
#define __SERVERDEFINITIONS_H__
#ifdef DLLSERVER_EX
	#ifdef __linux__
		#define DLLSERVER __attribute__((visibility("default")))
	#else
		#define DLLSERVER  __declspec(dllexport)   // export DLL information
	#endif
#else
	#ifdef __linux__
		#define DLLSERVER
	#else
		#define DLLSERVER  __declspec(dllimport)   // import DLL information
	#endif
#endif
#endif