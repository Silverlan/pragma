/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/entities/player.h"
#include "pragma/networking/netmessages.h"
#include "pragma/networking/standard_server.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/networking/master_server.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/networking/error.hpp>
#include <servermanager/interface/sv_nwm_manager.hpp>

extern DLLNETWORK ServerMessageMap *g_NetMessagesSv;

pragma::networking::IServer *ServerState::GetServer() {return m_server.get();}
pragma::networking::MasterServerRegistration *ServerState::GetMasterServerRegistration() {return m_serverReg.get();}
bool ServerState::IsServerRunning() const {return m_server && m_server->IsRunning();}
unsigned int ServerState::GetClientMessageID(std::string identifier)
{
	ClientMessageMap *map = GetClientMessageMap();
	return map->GetNetMessageID(identifier);
}

ServerMessageMap *ServerState::GetNetMessageMap() {return g_NetMessagesSv;}
SVNetMessage *ServerState::GetNetMessage(unsigned int ID)
{
	ServerMessageMap *map = GetNetMessageMap();
	return map->GetNetMessage(ID);
}

void ServerState::UpdatePlayerScore(pragma::SPlayerComponent &pl,int32_t score)
{
	auto *reg = GetMasterServerRegistration();
	auto *session = pl.GetClientSession();
	if(reg == nullptr || session == nullptr)
		return;
	reg->SetClientScore(session->GetSteamId(),score);
}

void ServerState::UpdatePlayerName(pragma::SPlayerComponent &pl,const std::string &name)
{
	auto *reg = GetMasterServerRegistration();
	auto *session = pl.GetClientSession();
	if(reg == nullptr || session == nullptr)
		return;
	reg->SetClientName(session->GetSteamId(),name);
}

void ServerState::DropClient(pragma::networking::IServerClient &session,pragma::networking::DropReason reason)
{
	auto *pl = session.GetPlayer();
	session.ClearResourceTransfer();

	auto *reg = GetMasterServerRegistration();
	if(reg)
		reg->DropClient(session.GetSteamId());

	pragma::networking::Error err;
	if(session.Drop(reason,err) == false)
		Con::cwar<<"WARNING: An error has occurred trying to drop client: '"<<err.GetMessage()<<"'!"<<Con::endl;
	if(pl == nullptr)
		return;
	auto *game = GetGameState();
	auto &ent = pl->GetEntity();
	NetPacket packet;
	packet->Write<unsigned int>(ent.GetIndex());
	game->RemoveEntity(&ent);
	SendPacket("playerdisconnect",packet,pragma::networking::Protocol::SlowReliable);
}
pragma::networking::IServerClient *ServerState::GetLocalClient() {return m_localClient.get();}

void ServerState::HandleLuaNetPacket(pragma::networking::IServerClient &session,NetPacket &packet)
{
	if(!IsGameActive())
		return;
	SGame *game = static_cast<SGame*>(GetGameState());
	game->HandleLuaNetPacket(session,packet);
}

static bool check_message_id(uint32_t id,const std::string &name)
{
	assert(id != 0);
	if(id == 0)
	{
		Con::csv<<"WARNING: Attempted to send unregistered message '"<<name<<"'!"<<Con::endl;
		return false;
	}
	return true;
}

void ServerState::SendPacket(const std::string &name,NetPacket &packet,pragma::networking::Protocol protocol,const pragma::networking::ClientRecipientFilter &rf)
{
	auto ID = GetClientMessageID(name);
	if(check_message_id(ID,name) == false || m_server == nullptr)
		return;
	packet.SetMessageID(ID);

	pragma::networking::Error err;
	if(m_server->SendPacket(protocol,packet,rf,err) == true)
		return;
	Con::cwar<<"WARNING: Unable to broadcast packet "<<ID<<": "<<err.GetMessage()<<Con::endl;
}
void ServerState::SendPacket(const std::string &name,NetPacket &packet,pragma::networking::Protocol protocol)
{
	SendPacket(name,packet,protocol,pragma::networking::ClientRecipientFilter{});
}
void ServerState::SendPacket(const std::string &name,NetPacket &packet)
{
	SendPacket(name,packet,pragma::networking::Protocol::FastUnreliable);
}
void ServerState::SendPacket(const std::string &name,pragma::networking::Protocol protocol)
{
	NetPacket packet{};
	SendPacket(name,packet,protocol);
}
