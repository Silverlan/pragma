/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WMS_SHAREDDEFINITIONS_H__
#define __WMS_SHAREDDEFINITIONS_H__


#ifdef WMS_STATIC
	#define DLLWMSSHARED
#elif WMS_DLL
	#ifdef __linux__
		#define DLLWMSSHARED __attribute__((visibility("default")))
	#else
		#define DLLWMSSHARED __declspec(dllexport)
	#endif
#else
	#ifdef __linux__
		#define DLLWMSSHARED
	#else
		#define DLLWMSSHARED __declspec(dllimport)
	#endif
#endif

#endif
