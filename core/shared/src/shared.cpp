/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"

// Link Libraries
#pragma comment(lib,"materialsystem.lib")
#pragma comment(lib,"luasystem.lib")
#pragma comment(lib,"networkmanager.lib")
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
#pragma comment(lib,"sharedutils.lib")
#pragma comment(lib,"mathutil.lib")
//#pragma comment(lib,"libnoise.lib")
#pragma comment(lib,"util_pad.lib")
//#pragma comment(lib,"VTFLib.lib")
#pragma comment(lib,"util_source_script.lib")
#pragma comment(lib,"ogg.lib")
//#pragma comment(lib,"RectangleBinPack.lib")

// Poly2tri
#pragma comment(lib,"clip2tri-static.lib")
#pragma comment(lib,"clipper-static.lib")
#pragma comment(lib,"poly2tri-static.lib")
//
