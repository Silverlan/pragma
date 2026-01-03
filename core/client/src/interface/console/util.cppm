// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:console.util;

export import pragma.shared;

export namespace pragma::console {
	DLLCLIENT ConVarHandle get_client_con_var(std::string scmd);
}
