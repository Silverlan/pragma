// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:console.util;

export import pragma.shared;

export namespace pragma::console {
	DLLSERVER ConVarHandle get_server_con_var(std::string scmd);
}
