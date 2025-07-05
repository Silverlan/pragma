// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/console/c_cvar.h"
#include "pragma/console/convarhandle.h"
#include "pragma/clientstate/clientstate.h"

extern ClientState *client;
ConVarHandle GetClientConVar(std::string scmd) { return client->GetConVarHandle(scmd); }
