/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_CVAR_H__
#define __S_CVAR_H__

#include <string>
#include "pragma/serverdefinitions.h"
#include "pragma/console/convarhandle.h"

DLLSERVER ConVarHandle GetServerConVar(std::string scmd);

#endif