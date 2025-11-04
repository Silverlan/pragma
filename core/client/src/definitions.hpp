// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#pragma once

#ifdef DLLCLIENT_EX
#ifdef __linux__
#define DLLCLIENT __attribute__((visibility("default")))
#else
#define DLLCLIENT __declspec(dllexport) // export DLL information
#endif
#else
#ifdef __linux__
#define DLLCLIENT
#else
#define DLLCLIENT __declspec(dllimport) // import DLL information
#endif
#endif
