#ifndef __C_ENGINEDEFINITIONS_H__
#define __C_ENGINEDEFINITIONS_H__
#ifdef DLLCENGINE_EX
	#ifdef __linux__
		#define DLLCENGINE __attribute__((visibility("default")))
	#else
		#define DLLCENGINE  __declspec(dllexport)   // export DLL information
	#endif
#else
	#ifdef __linux__
		#define DLLCENGINE
	#else
		#define DLLCENGINE  __declspec(dllimport)   // import DLL information
	#endif
#endif
#endif