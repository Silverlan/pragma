// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :console.util;

import :server_state;

pragma::console::ConVarHandle pragma::console::get_server_con_var(std::string scmd) { return ServerState::Get()->GetConVarHandle(scmd); }
