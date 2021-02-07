/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/console/engine_cvar.h"
#include <pragma/console/convarhandle.h>
#include "pragma/engine.h"

extern DLLNETWORK Engine *engine;
ConVarHandle GetEngineConVar(std::string scmd)
{
	return engine->GetConVarHandle(scmd);
}