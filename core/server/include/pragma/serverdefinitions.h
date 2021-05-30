/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __SERVERDEFINITIONS_H__
#define __SERVERDEFINITIONS_H__
#ifdef DLLSERVER_EX
	#ifdef __linux__
		#define DLLSERVER __attribute__((visibility("default")))
	#else
		#define DLLSERVER  __declspec(dllexport)   // export DLL information
	#endif
#else
	#ifdef __linux__
		#define DLLSERVER
	#else
		#define DLLSERVER  __declspec(dllimport)   // import DLL information
	#endif
#endif
#endif