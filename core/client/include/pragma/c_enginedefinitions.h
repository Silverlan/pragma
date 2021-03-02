/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENGINEDEFINITIONS_H__
#define __C_ENGINEDEFINITIONS_H__
#ifdef DLLCLIENT_EX
	#ifdef __linux__
		#define DLLCLIENT __attribute__((visibility("default")))
	#else
		#define DLLCLIENT  __declspec(dllexport)   // export DLL information
	#endif
#else
	#ifdef __linux__
		#define DLLCLIENT
	#else
		#define DLLCLIENT  __declspec(dllimport)   // import DLL information
	#endif
#endif
#endif