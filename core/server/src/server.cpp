#include "stdafx_server.h"

// Link Libraries
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"materialsystem.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"datasystem.lib")
#ifdef USE_LUAJIT
	#pragma comment(lib,"lua51.lib")
#else
	#pragma comment(lib,"lua530.lib")
#endif
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"bz2.lib")
#pragma comment(lib,"shared.lib")
#pragma comment(lib,"engine.lib")
#pragma comment(lib,"recast.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"util_sound.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"networkmanager.lib")
#pragma comment(lib,"servermanager.lib")
#pragma comment(lib,"clientmanager.lib")
#pragma comment(lib,"wms_shared.lib")
//
