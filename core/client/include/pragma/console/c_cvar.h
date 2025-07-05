// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_CVAR_H__
#define __C_CVAR_H__

#include <string>
#include "pragma/console/convarhandle.h"
#include "pragma/clientdefinitions.h"

DLLCLIENT ConVarHandle GetClientConVar(std::string scmd);

#endif
