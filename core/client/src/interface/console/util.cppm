// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <string>


export module pragma.client:console.util;

export import pragma.shared;

export DLLCLIENT ConVarHandle GetClientConVar(std::string scmd);
