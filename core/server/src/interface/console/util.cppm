// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.server:console.util;

export import pragma.shared;

export DLLSERVER ConVarHandle GetServerConVar(std::string scmd);
