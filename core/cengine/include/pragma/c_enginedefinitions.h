/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_ENGINEDEFINITIONS_H__
#define __C_ENGINEDEFINITIONS_H__
#ifdef DLLCENGINE_EX
	#ifdef __linux__
		#define DLLCENGINE __attribute__((visibility("default")))
	#else
		#define DLLCENGINE  __declspec(dllexport)   // export DLL information
	#endif
#else
	#ifdef __linux__
		#define DLLCENGINE
	#else
		#define DLLCENGINE  __declspec(dllimport)   // import DLL information
	#endif
#endif
#endif