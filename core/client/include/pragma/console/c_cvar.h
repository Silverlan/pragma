/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_CVAR_H__
#define __C_CVAR_H__

#include <string>
#include "pragma/console/convarhandle.h"
#include "pragma/clientdefinitions.h"

DLLCLIENT ConVarHandle GetClientConVar(std::string scmd);

#endif