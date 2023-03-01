/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/console/c_cvar.h"
#include "pragma/console/convarhandle.h"
#include "pragma/clientstate/clientstate.h"

extern ClientState *client;
ConVarHandle GetClientConVar(std::string scmd) { return client->GetConVarHandle(scmd); }
