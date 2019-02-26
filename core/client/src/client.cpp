#include "stdafx_client.h"

// Link Libraries
#pragma comment(lib,"prosper.lib")
#pragma comment(lib,"Anvil.lib")
#pragma comment(lib,"vulkan-1.lib")
#pragma comment(lib,"datasystem.lib")
#pragma comment(lib,"cengine.lib")
#pragma comment(lib,"cmaterialsystem.lib")
#pragma comment(lib,"materialsystem.lib")
#pragma comment(lib,"networkmanager.lib")
#pragma comment(lib,"clientmanager.lib")
#pragma comment(lib,"wgui.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"glfw3dll.lib")
#pragma comment(lib,"shared.lib")
#pragma comment(lib,"engine.lib")
#pragma comment(lib,"iglfw.lib")
#pragma comment(lib,"RectangleBinPack.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"libnoise.lib")
#pragma comment(lib,"alsoundsystem.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"bzip2.lib")
#pragma comment(lib,"recast.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"util_sound.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"wms_shared.lib")
#pragma comment(lib,"util_pad.lib")
#pragma comment(lib,"util_timeline_scene.lib")
#pragma comment(lib,"util_source_script.lib")
#ifdef _DEBUG
	#ifdef PHYS_ENGINE_BULLET
		#pragma comment(lib,"BulletDynamics_Debug.lib")
		#pragma comment(lib,"BulletCollision_Debug.lib")
		#pragma comment(lib,"LinearMath_Debug.lib")
	#endif
#else
	#ifdef PHYS_ENGINE_BULLET
		#pragma comment(lib,"BulletDynamics.lib")
		#pragma comment(lib,"BulletCollision.lib")
		#pragma comment(lib,"LinearMath.lib")
	#endif
#endif
//