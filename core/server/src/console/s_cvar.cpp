// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"

import pragma.server.server_state;

ConVarHandle GetServerConVar(std::string scmd) { return ServerState::Get()->GetConVarHandle(scmd); }
