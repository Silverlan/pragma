// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"

module pragma.server;
import :console.util;

import :server_state;

ConVarHandle GetServerConVar(std::string scmd) { return ServerState::Get()->GetConVarHandle(scmd); }
