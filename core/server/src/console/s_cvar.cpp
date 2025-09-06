// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/console/s_cvar.h"
#include "pragma/console/convarhandle.h"

import pragma.server.server_state;

extern ServerState *server;
ConVarHandle GetServerConVar(std::string scmd) { return server->GetConVarHandle(scmd); }
