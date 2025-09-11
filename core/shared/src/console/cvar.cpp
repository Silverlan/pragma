// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/console/cvar.h"
#include <pragma/console/convarhandle.h>
#include "pragma/engine.h"
#include <pragma/console/convars.h>

extern Engine *engine;
ConVarHandle GetConVar(std::string scmd) { return Engine::Get()->GetConVarHandle(scmd); }
