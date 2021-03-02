/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __ENGINE_CVAR_H__
#define __ENGINE_CVAR_H__

#include <string>
#include <pragma/console/convarhandle.h>
#include "pragma/definitions.h"
#include <pragma/console/convars.h>

DLLNETWORK ConVarHandle GetEngineConVar(std::string scmd);

#endif