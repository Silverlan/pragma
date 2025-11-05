// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :console.common;

extern pragma::Engine *engine;
ConVarHandle GetConVar(std::string scmd) { return pragma::Engine::Get()->GetConVarHandle(scmd); }
ConVarHandle GetEngineConVar(std::string scmd) { return pragma::Engine::Get()->GetConVarHandle(scmd); }
