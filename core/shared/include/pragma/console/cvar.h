// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __CVAR_H__
#define __CVAR_H__

#include <string>
#include "pragma/definitions.h"
#include <pragma/console/convarhandle.h>

DLLNETWORK ConVarHandle GetConVar(std::string scmd);

#endif
