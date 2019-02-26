#ifndef __WVMODULE_H__
#define __WVMODULE_H__

#ifdef MODULE_SERVER
	#include "iserver.h"
	#define IState IServer
	#pragma comment(lib,"IServer.lib")
#else
#ifdef MODULE_CLIENT
	#include "iclient.h"
	#define IState IClient
	#pragma comment(lib,"IClient.lib")
#endif
#endif

#endif