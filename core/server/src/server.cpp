#include "stdafx_server.h"

// Link Libraries
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"materialsystem.lib")
#ifdef _DEBUG
	#pragma comment(lib,"boost_system-vc141-mt-gd-x64-1_69")
#else
	#pragma comment(lib,"boost_system-vc141-mt-x64-1_69")
#endif
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"datasystem.lib")
#ifdef USE_LUAJIT
	#pragma comment(lib,"lua51.lib")
#else
	#pragma comment(lib,"lua530.lib")
#endif
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"bzip2.lib")
#pragma comment(lib,"shared.lib")
#pragma comment(lib,"engine.lib")
#pragma comment(lib,"recast.lib")
#pragma comment(lib,"util.lib")
#pragma comment(lib,"util_sound.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"networkmanager.lib")
#pragma comment(lib,"servermanager.lib")
#pragma comment(lib,"clientmanager.lib")
#pragma comment(lib,"wms_shared.lib")
#ifdef _DEBUG
#ifdef PHYS_ENGINE_PHYSX
	#pragma comment(lib,"PhysX3CHECKED_x86.lib")
	#pragma comment(lib,"PhysX3CommonCHECKED_x86.lib")
	#pragma comment(lib,"PhysX3ExtensionsCHECKED.lib")
#endif
#else
#ifdef PHYS_ENGINE_PHYSX
	#pragma comment(lib,"PhysX3_x86.lib")
	#pragma comment(lib,"PhysX3Common_x86.lib")
	#pragma comment(lib,"PhysX3Extensions.lib")
#endif
#endif
//