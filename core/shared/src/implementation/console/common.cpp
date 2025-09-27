// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/console/cvar.h"
#include <pragma/console/convarhandle.h>
#include "pragma/engine.h"
#include <pragma/console/convars.h>

module pragma.shared;

import :console.common;

extern Engine *engine;
ConVarHandle GetConVar(std::string scmd) { return Engine::Get()->GetConVarHandle(scmd); }
ConVarHandle GetEngineConVar(std::string scmd) { return Engine::Get()->GetConVarHandle(scmd); }
