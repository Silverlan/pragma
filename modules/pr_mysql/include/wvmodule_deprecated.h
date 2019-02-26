#ifdef __WVMODULE_H__
#define __WVMODULE_H__

#ifdef MODULE_SERVER
	//#define IState IServer
	//#pragma comment(lib,"IServer.lib")
#else
#error "Deprecated!"
	#define IState IClient
	#pragma comment(lib,"IClient.lib")
#endif

#endif