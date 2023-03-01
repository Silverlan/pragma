/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/iserverstate.hpp"
#include <sharedutils/util_library.hpp>
#include <sharedutils/util_path.hpp>

void pragma::IServerState::Initialize(util::Library &lib)
{
	create_server_state = lib.FindSymbolAddress<decltype(create_server_state)>("pr_sv_create_server_state");
	start_server = lib.FindSymbolAddress<decltype(start_server)>("pr_sv_start_server");
	close_server = lib.FindSymbolAddress<decltype(close_server)>("pr_sv_close_server");
	is_server_running = lib.FindSymbolAddress<decltype(is_server_running)>("pr_sv_is_server_running");
	get_server_steam_id = lib.FindSymbolAddress<decltype(get_server_steam_id)>("pr_sv_get_server_steam_id");
	get_server_state = lib.FindSymbolAddress<decltype(get_server_state)>("pr_sv_get_server_state");
	clear_server_state = lib.FindSymbolAddress<decltype(clear_server_state)>("pr_sv_clear_server_state");
	handle_local_host_player_server_packet = lib.FindSymbolAddress<decltype(handle_local_host_player_server_packet)>("pr_sv_handle_local_host_player_server_packet");
	connect_local_host_player_client = lib.FindSymbolAddress<decltype(connect_local_host_player_client)>("pr_sv_connect_local_host_player_client");
	m_bValid = start_server && close_server && is_server_running && get_server_steam_id && get_server_state && clear_server_state && handle_local_host_player_server_packet && connect_local_host_player_client;
}
