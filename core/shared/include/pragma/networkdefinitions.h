// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __NETWORKDEFINITIONS_H__
#define __NETWORKDEFINITIONS_H__
#ifdef DLLNETWORK_EX
#ifdef __linux__
#define DLLNETWORK __attribute__((visibility("default")))
#else
#define DLLNETWORK __declspec(dllexport) // export DLL information
#endif
#else
#ifdef __linux__
#define DLLNETWORK
#else
#define DLLNETWORK __declspec(dllimport) // import DLL information
#endif
#endif
#endif
