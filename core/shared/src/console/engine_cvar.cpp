// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/console/engine_cvar.h"
#include <pragma/console/convarhandle.h>
#include "pragma/engine.h"

ConVarHandle GetEngineConVar(std::string scmd) { return Engine::Get()->GetConVarHandle(scmd); }
