// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
