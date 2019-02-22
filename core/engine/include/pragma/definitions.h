#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__
#ifdef DLLENGINE_EX
	#ifdef __linux__
		#define DLLENGINE __attribute__((visibility("default")))
	#else
		#define DLLENGINE  __declspec(dllexport)   // export DLL information
	#endif
#else
	#ifdef __linux__
		#define DLLENGINE
	#else
		#define DLLENGINE  __declspec(dllimport)   // import DLL information
	#endif
#endif
#endif