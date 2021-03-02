/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/console/s_cvar.h"
#include "pragma/console/convarhandle.h"
#include <pragma/serverstate/serverstate.h>

extern ServerState *server;
ConVarHandle GetServerConVar(std::string scmd)
{
	return server->GetConVarHandle(scmd);
}