// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :networking.net_message_map;

pragma::networking::ClientMessageMap *pragma::networking::get_client_message_map()
{
	static ClientMessageMap g_clientMessageMap;
	return &g_clientMessageMap;
}
pragma::networking::ServerMessageMap *pragma::networking::get_server_message_map()
{
	static ServerMessageMap g_serverMessageMap;
	return &g_serverMessageMap;
}
