#ifndef __WVMODULE_H__
#define __WVMODULE_H__

#ifdef MODULE_SERVER
	#include "pragma/iserver.h"
	#define IState iserver
	//#pragma comment(lib,"IServer.lib")
#else
#ifdef MODULE_CLIENT
	#include "pragma/iclient.h"
	#define IState iclient
	//#pragma comment(lib,"IClient.lib")
#endif
#endif

#endif