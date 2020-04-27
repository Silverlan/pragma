/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__
#ifdef DLLENGINE_EX
	#ifdef __linux__
		#define DLLENGINE __attribute__((visibility("default")))
	#else
		#define DLLENGINE  __declspec(dllexport)   // export DLL information
	#endif
#else
	#ifdef __linux__
		#define DLLENGINE
	#else
		#define DLLENGINE  __declspec(dllimport)   // import DLL information
	#endif
#endif
#endif