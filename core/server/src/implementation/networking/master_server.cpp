// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :networking.master_server;

std::unique_ptr<pragma::networking::MasterServerRegistration> pragma::networking::MasterServerRegistration::Register(const util::Library &steamworksLibrary, const GameServerInfo &serverInfo)
{
	std::unique_ptr<MasterServerRegistration> reg {new MasterServerRegistration {}};
	if(reg->Initialize(steamworksLibrary) == false || reg->m_register_server(serverInfo) == false)
		return nullptr;
	CallbackEvents events {};
	auto *pReg = reg.get();
	events.onAuthCompleted = [pReg](uint64_t steamId, bool authSuccess) {
		if(pReg->m_callbackEvents.onAuthCompleted)
			pReg->m_callbackEvents.onAuthCompleted(steamId, authSuccess);
	};
	reg->m_steamworks_set_master_server_callback_events(events);
	return reg;
}
pragma::networking::MasterServerRegistration::~MasterServerRegistration()
{
	if(m_unregister_server)
		m_unregister_server();
}
bool pragma::networking::MasterServerRegistration::Initialize(const util::Library &library)
{
	m_register_server = library.FindSymbolAddress<decltype(m_register_server)>("pr_steamworks_register_server");
	m_unregister_server = library.FindSymbolAddress<decltype(m_unregister_server)>("pr_steamworks_unregister_server");
	m_update_server_data = library.FindSymbolAddress<decltype(m_update_server_data)>("pr_steamworks_update_server_data");
	m_steamworks_get_server_data = library.FindSymbolAddress<decltype(m_steamworks_get_server_data)>("pr_steamworks_get_server_data");

	m_steamworks_authenticate_and_add_client = library.FindSymbolAddress<decltype(m_steamworks_authenticate_and_add_client)>("pr_steamworks_authenticate_and_add_client");
	m_steamworks_drop_client = library.FindSymbolAddress<decltype(m_steamworks_drop_client)>("pr_steamworks_drop_client");
	m_steamworks_set_client_name = library.FindSymbolAddress<decltype(m_steamworks_set_client_name)>("pr_steamworks_set_client_name");
	m_steamworks_set_client_score = library.FindSymbolAddress<decltype(m_steamworks_set_client_score)>("pr_steamworks_set_client_score");
	m_steamworks_set_master_server_callback_events = library.FindSymbolAddress<decltype(m_steamworks_set_master_server_callback_events)>("pr_steamworks_set_master_server_callback_events");

	return m_register_server && m_unregister_server && m_update_server_data && m_steamworks_get_server_data && m_steamworks_authenticate_and_add_client && m_steamworks_drop_client && m_steamworks_set_client_name && m_steamworks_set_client_score
	  && m_steamworks_set_master_server_callback_events;
}
void pragma::networking::MasterServerRegistration::UpdateServerData() const { m_update_server_data(); }
pragma::networking::GameServerInfo &pragma::networking::MasterServerRegistration::GetServerInfo()
{
	GameServerInfo *serverInfo;
	m_steamworks_get_server_data(&serverInfo);
	return *serverInfo;
}
void pragma::networking::MasterServerRegistration::SetCallbackEvents(const CallbackEvents &callbackEvents) { m_callbackEvents = callbackEvents; }
void pragma::networking::MasterServerRegistration::AuthenticateAndAddClient(uint64_t steamId, std::vector<char> &token, const std::string &clientName) { m_steamworks_authenticate_and_add_client(steamId, token, clientName); }
void pragma::networking::MasterServerRegistration::DropClient(SteamId steamId) { m_steamworks_drop_client(steamId); }
void pragma::networking::MasterServerRegistration::SetClientName(SteamId steamId, const std::string &name) { m_steamworks_set_client_name(steamId, name); }
void pragma::networking::MasterServerRegistration::SetClientScore(SteamId steamId, int32_t score) { m_steamworks_set_client_score(steamId, score); }
