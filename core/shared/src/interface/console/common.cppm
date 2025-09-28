// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <string>

export module pragma.shared:console.common;

import :console.convar_handle;

export {
	DLLNETWORK ConVarHandle GetConVar(std::string scmd);
	DLLNETWORK ConVarHandle GetEngineConVar(std::string scmd);
};
