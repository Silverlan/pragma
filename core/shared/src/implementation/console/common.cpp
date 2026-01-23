// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :console.common;

extern pragma::Engine *engine;
pragma::console::ConVarHandle pragma::console::get_con_var(std::string scmd) { return Engine::GetConVarHandle(scmd); }
pragma::console::ConVarHandle pragma::console::get_engine_con_var(std::string scmd) { return Engine::GetConVarHandle(scmd); }
