// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.server;
import :server_state;

import :entities.components;
import pragma.shared;

#undef GetMessage

bool pragma::ServerState::HandlePacket(networking::IServerClient &session, NetPacket &packet)
{
	unsigned int ID = packet.GetMessageID();
	networking::SVNetMessage *msg = GetNetMessage(ID);
	if(msg == nullptr)
		return false;
	msg->handler(static_cast<networking::ServerClientHandle>(&session), packet);
	return true;
}

pragma::networking::IServer *pragma::ServerState::GetServer() { return m_server.get(); }
pragma::networking::MasterServerRegistration *pragma::ServerState::GetMasterServerRegistration() { return m_serverReg.get(); }
bool pragma::ServerState::IsServerRunning() const { return m_server && m_server->IsRunning(); }
unsigned int pragma::ServerState::GetClientMessageID(std::string identifier)
{
	auto *map = networking::get_client_message_map();
	return map->GetNetMessageID(identifier);
}

pragma::networking::ServerMessageMap *pragma::ServerState::GetNetMessageMap() { return networking::get_server_message_map(); }
pragma::networking::SVNetMessage *pragma::ServerState::GetNetMessage(unsigned int ID)
{
	auto *map = GetNetMessageMap();
	return map->GetNetMessage(ID);
}

void pragma::ServerState::UpdatePlayerScore(SPlayerComponent &pl, int32_t score)
{
	auto *reg = GetMasterServerRegistration();
	auto *session = pl.GetClientSession();
	if(reg == nullptr || session == nullptr)
		return;
	reg->SetClientScore(session->GetSteamId(), score);
}

void pragma::ServerState::UpdatePlayerName(SPlayerComponent &pl, const std::string &name)
{
	auto *reg = GetMasterServerRegistration();
	auto *session = pl.GetClientSession();
	if(reg == nullptr || session == nullptr)
		return;
	reg->SetClientName(session->GetSteamId(), name);
}

void pragma::ServerState::DropClient(networking::IServerClient &session, networking::DropReason reason)
{
	auto *pl = session.GetPlayer();
	session.ClearResourceTransfer();

	auto *reg = GetMasterServerRegistration();
	if(reg)
		reg->DropClient(session.GetSteamId());

	networking::Error err;
	if(session.Drop(reason, err) == false)
		Con::CWAR << "An error has occurred trying to drop client: '" << err.GetMessage() << "'!" << Con::endl;
	if(pl == nullptr)
		return;
	auto *game = GetGameState();
	auto &ent = pl->GetEntity();
	NetPacket packet;
	packet->Write<unsigned int>(ent.GetIndex());
	game->RemoveEntity(&ent);
	// SendPacket("playerdisconnect", packet, pragma::networking::Protocol::SlowReliable);
}
pragma::networking::IServerClient *pragma::ServerState::GetLocalClient() { return m_localClient.get(); }

void pragma::ServerState::HandleLuaNetPacket(networking::IServerClient &session, NetPacket &packet)
{
	if(!IsGameActive())
		return;
	SGame *game = static_cast<SGame *>(GetGameState());
	game->HandleLuaNetPacket(session, packet);
}

static bool check_message_id(uint32_t id, const std::string &name)
{
	assert(id != 0);
	if(id == 0) {
		Con::CWAR << Con::PREFIX_SERVER << "Attempted to send unregistered message '" << name << "'!" << Con::endl;
		return false;
	}
	return true;
}

void pragma::ServerState::SendPacket(const std::string &name, NetPacket &packet, networking::Protocol protocol, const networking::ClientRecipientFilter &rf)
{
	auto ID = GetClientMessageID(name);
	if(check_message_id(ID, name) == false || m_server == nullptr)
		return;
	packet.SetMessageID(ID);

	networking::Error err;
	if(m_server->SendPacket(protocol, packet, rf, err) == true)
		return;
	Con::CWAR << "Unable to broadcast packet " << ID << ": " << err.GetMessage() << Con::endl;
}
void pragma::ServerState::SendPacket(const std::string &name, NetPacket &packet, networking::Protocol protocol) { SendPacket(name, packet, protocol, networking::ClientRecipientFilter {}); }
void pragma::ServerState::SendPacket(const std::string &name, NetPacket &packet) { SendPacket(name, packet, networking::Protocol::FastUnreliable); }
void pragma::ServerState::SendPacket(const std::string &name, networking::Protocol protocol)
{
	NetPacket packet {};
	SendPacket(name, packet, protocol);
}
