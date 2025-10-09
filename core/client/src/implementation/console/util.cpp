// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :console.util;

ConVarHandle GetClientConVar(std::string scmd) { return pragma::get_client_state()->GetConVarHandle(scmd); }
