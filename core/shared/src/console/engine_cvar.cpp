// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/console/engine_cvar.h"
#include <pragma/console/convarhandle.h>
#include "pragma/engine.h"

extern DLLNETWORK Engine *engine;
ConVarHandle GetEngineConVar(std::string scmd) { return engine->GetConVarHandle(scmd); }
