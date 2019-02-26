#include "stdafx_shared.h"

// Link Libraries
#pragma comment(lib,"materialsystem.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"networkmanager.lib")
#pragma comment(lib,"vfilesystem.lib")
#pragma comment(lib,"datasystem.lib")
#pragma comment(lib,"engine.lib")
#ifdef USE_LUAJIT
	#pragma comment(lib,"lua51.lib")
#else
	#pragma comment(lib,"lua530.lib")
#endif
#pragma comment(lib,"luabind.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"bzip2.lib")
#pragma comment(lib,"libvorbisfile.lib")
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
//#pragma comment(lib,"libnoise.lib")
#pragma comment(lib,"util_pad.lib")
#pragma comment(lib,"util_bsp.lib")
#pragma comment(lib,"util_vmf.lib")
#pragma comment(lib,"VTFLib.lib")
#pragma comment(lib,"util_source_script.lib")
#pragma comment(lib,"util_mmd.lib")
#pragma comment(lib,"util_fgd.lib")
#pragma comment(lib,"RectangleBinPack.lib")

// Poly2tri
#pragma comment(lib,"clip2tri-static.lib")
#pragma comment(lib,"clipper-static.lib")
#pragma comment(lib,"poly2tri-static.lib")

#ifdef _DEBUG
#ifdef PHYS_ENGINE_BULLET
	#pragma comment(lib,"BulletDynamics_Debug.lib")
	#pragma comment(lib,"BulletCollision_Debug.lib")
	#pragma comment(lib,"BulletSoftBody_Debug.lib")
	#pragma comment(lib,"LinearMath_Debug.lib")
#endif
#ifdef PHYS_ENGINE_PHYSX
	#pragma comment(lib,"PhysX3CHECKED_x86.lib")
	#pragma comment(lib,"PhysX3CharacterKinematicCHECKED_x86.lib")
	#pragma comment(lib,"PhysX3CommonCHECKED_x86.lib")
	#pragma comment(lib,"PhysX3ExtensionsCHECKED.lib")
	#pragma comment(lib,"PhysX3VehicleCHECKED.lib")
#endif
#else
#ifdef PHYS_ENGINE_BULLET
	#pragma comment(lib,"BulletDynamics.lib")
	#pragma comment(lib,"BulletCollision.lib")
	#pragma comment(lib,"BulletSoftBody.lib")
	#pragma comment(lib,"LinearMath.lib")
#elif PHYS_ENGINE_PHYSX
	#pragma comment(lib,"PhysX3_x86.lib")
	#pragma comment(lib,"PhysX3CharacterKinematic_x86.lib")
	#pragma comment(lib,"PhysX3Common_x86.lib")
	#pragma comment(lib,"PhysX3Extensions.lib")
	#pragma comment(lib,"PhysX3Vehicle.lib")
#endif
#endif
//
