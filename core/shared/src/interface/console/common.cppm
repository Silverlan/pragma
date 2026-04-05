// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:console.common;

import :console.convar_handle;

export namespace pragma::console {
	DLLNETWORK ConVarHandle get_con_var(std::string_view scmd);
	DLLNETWORK ConVarHandle get_engine_con_var(std::string_view scmd);

	class CVarHandler;
	DLLNETWORK size_t get_con_var_value_bytes(CVarHandler &cvarHandler, std::string_view scmd, std::string_view defaultValue);
};
