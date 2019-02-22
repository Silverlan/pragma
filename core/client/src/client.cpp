#include "stdafx_client.h"

// Link Libraries
#pragma comment(lib,"prosper.lib")
#pragma comment(lib,"Anvil.lib")
#pragma comment(lib,"vulkan-1.lib")
#pragma comment(lib,"shaderinfo.lib")
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
#pragma comment(lib,"glfw.lib")
#pragma comment(lib,"RectangleBinPack.lib")
#ifdef USE_LUAJIT
	#pragma comment(lib,"lua51.lib")
#else
	#pragma comment(lib,"lua530.lib")
#endif
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"libnoise.lib")
#pragma comment(lib,"alsoundsystem.lib")
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"bzip2.lib")
#pragma comment(lib,"recast.lib")
#pragma comment(lib,"libvorbisfile.lib")
#pragma comment(lib,"util.lib")
#pragma comment(lib,"util_sound.lib")
#pragma comment(lib,"mathutil.lib")
#pragma comment(lib,"wms_shared.lib")
#pragma comment(lib,"pad.lib")
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
		#pragma comment(lib,"BulletDynamics_RelWithDebugInfo.lib")
		#pragma comment(lib,"BulletCollision_RelWithDebugInfo.lib")
		#pragma comment(lib,"LinearMath_RelWithDebugInfo.lib")
	#endif
#endif
//