// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"

module pragma.server.server_state;

import pragma.server.entities.components;

bool ServerState::HandlePacket(pragma::networking::IServerClient &session, NetPacket &packet)
{
	unsigned int ID = packet.GetMessageID();
	SVNetMessage *msg = GetNetMessage(ID);
	if(msg == nullptr)
		return false;
	msg->handler(session, packet);
	return true;
}

pragma::networking::IServer *ServerState::GetServer() { return m_server.get(); }
pragma::networking::MasterServerRegistration *ServerState::GetMasterServerRegistration() { return m_serverReg.get(); }
bool ServerState::IsServerRunning() const { return m_server && m_server->IsRunning(); }
unsigned int ServerState::GetClientMessageID(std::string identifier)
{
	ClientMessageMap *map = GetClientMessageMap();
	return map->GetNetMessageID(identifier);
}

ServerMessageMap *ServerState::GetNetMessageMap() { return GetServerMessageMap(); }
SVNetMessage *ServerState::GetNetMessage(unsigned int ID)
{
	ServerMessageMap *map = GetNetMessageMap();
	return map->GetNetMessage(ID);
}

void ServerState::UpdatePlayerScore(pragma::SPlayerComponent &pl, int32_t score)
{
	auto *reg = GetMasterServerRegistration();
	auto *session = pl.GetClientSession();
	if(reg == nullptr || session == nullptr)
		return;
	reg->SetClientScore(session->GetSteamId(), score);
}

void ServerState::UpdatePlayerName(pragma::SPlayerComponent &pl, const std::string &name)
{
	auto *reg = GetMasterServerRegistration();
	auto *session = pl.GetClientSession();
	if(reg == nullptr || session == nullptr)
		return;
	reg->SetClientName(session->GetSteamId(), name);
}

void ServerState::DropClient(pragma::networking::IServerClient &session, pragma::networking::DropReason reason)
{
	auto *pl = session.GetPlayer();
	session.ClearResourceTransfer();

	auto *reg = GetMasterServerRegistration();
	if(reg)
		reg->DropClient(session.GetSteamId());

	pragma::networking::Error err;
	if(session.Drop(reason, err) == false)
		Con::cwar << "An error has occurred trying to drop client: '" << err.GetMessage() << "'!" << Con::endl;
	if(pl == nullptr)
		return;
	auto *game = GetGameState();
	auto &ent = pl->GetEntity();
	NetPacket packet;
	packet->Write<unsigned int>(ent.GetIndex());
	game->RemoveEntity(&ent);
	SendPacket("playerdisconnect", packet, pragma::networking::Protocol::SlowReliable);
}
pragma::networking::IServerClient *ServerState::GetLocalClient() { return m_localClient.get(); }

void ServerState::HandleLuaNetPacket(pragma::networking::IServerClient &session, NetPacket &packet)
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
		Con::cwar << Con::PREFIX_SERVER << "Attempted to send unregistered message '" << name << "'!" << Con::endl;
		return false;
	}
	return true;
}

void ServerState::SendPacket(const std::string &name, NetPacket &packet, pragma::networking::Protocol protocol, const pragma::networking::ClientRecipientFilter &rf)
{
	auto ID = GetClientMessageID(name);
	if(check_message_id(ID, name) == false || m_server == nullptr)
		return;
	packet.SetMessageID(ID);

	pragma::networking::Error err;
	if(m_server->SendPacket(protocol, packet, rf, err) == true)
		return;
	Con::cwar << "Unable to broadcast packet " << ID << ": " << err.GetMessage() << Con::endl;
}
void ServerState::SendPacket(const std::string &name, NetPacket &packet, pragma::networking::Protocol protocol) { SendPacket(name, packet, protocol, pragma::networking::ClientRecipientFilter {}); }
void ServerState::SendPacket(const std::string &name, NetPacket &packet) { SendPacket(name, packet, pragma::networking::Protocol::FastUnreliable); }
void ServerState::SendPacket(const std::string &name, pragma::networking::Protocol protocol)
{
	NetPacket packet {};
	SendPacket(name, packet, protocol);
}
