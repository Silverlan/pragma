/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __NETWORKDEFINITIONS_H__
#define __NETWORKDEFINITIONS_H__
#ifdef DLLNETWORK_EX
	#ifdef __linux__
		#define DLLNETWORK __attribute__((visibility("default")))
	#else
		#define DLLNETWORK  __declspec(dllexport)   // export DLL information
	#endif
#else
	#ifdef __linux__
		#define DLLNETWORK
	#else
		#define DLLNETWORK  __declspec(dllimport)   // import DLL information
	#endif
#endif
#endif