// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENGINE_CVAR_H__
#define __ENGINE_CVAR_H__

#include <string>
#include <pragma/console/convarhandle.h>
#include "pragma/definitions.h"
#include <pragma/console/convars.h>

DLLNETWORK ConVarHandle GetEngineConVar(std::string scmd);

#endif
