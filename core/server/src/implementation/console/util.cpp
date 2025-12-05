// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.server;
import :console.util;

import :server_state;

ConVarHandle GetServerConVar(std::string scmd) { return pragma::ServerState::Get()->GetConVarHandle(scmd); }
