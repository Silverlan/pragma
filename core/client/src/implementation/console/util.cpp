// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :console.util;

pragma::console::ConVarHandle pragma::console::get_client_con_var(std::string scmd) { return get_client_state()->GetConVarHandle(scmd); }
