// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;



#include "pragma/networkdefinitions.h"

module pragma.shared;

import :networking.net_message_map;

pragma::networking::ClientMessageMap *GetClientMessageMap()
{
	static pragma::networking::ClientMessageMap g_clientMessageMap;
	return &g_clientMessageMap;
}
pragma::networking::ServerMessageMap *GetServerMessageMap()
{
	static pragma::networking::ServerMessageMap g_serverMessageMap;
	return &g_serverMessageMap;
}
