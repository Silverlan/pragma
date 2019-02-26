#ifndef __WVMODULE_H__
#define __WVMODULE_H__

#ifdef MODULE_SERVER
	#include "iserver.h"
	#define IState iserver
	//#pragma comment(lib,"IServer.lib")
#else
	#include "iclient.h"
	#define IState iclient
	//#pragma comment(lib,"IClient.lib")
#endif

#endif