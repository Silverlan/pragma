// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :util.server_state_interface;

void pragma::IServerState::Initialize(util::Library &lib)
{
	auto *register_server_entities = lib.FindSymbolAddress<void(*)()>("pr_sv_register_server_entities");
	assert(register_server_entities != nullptr);
	register_server_entities();

	auto *register_server_net_messages = lib.FindSymbolAddress<void(*)()>("pr_sv_register_server_net_messages");
	assert(register_server_net_messages != nullptr);
	register_server_net_messages();
	
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
