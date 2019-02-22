#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/networking/wvserverclient.h"
#include "pragma/networking/wvlocalclient.h"
#include "pragma/networking/wvserver.h"
#include "pragma/entities/player.h"
#include "pragma/networking/netmessages.h"
#include "pragma/entities/components/s_player_component.hpp"
#include <pragma/entities/components/base_player_component.hpp>
#include <servermanager/interface/sv_nwm_manager.hpp>

extern DLLNETWORK ServerMessageMap *g_NetMessagesSv;
WVServer *ServerState::GetServer() {return m_server.get();}
bool ServerState::IsServerRunning() const {return (m_server != nullptr) ? true : false;}
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

void ServerState::DropClient(WVServerClient *session)
{
	pragma::SPlayerComponent *pl = nullptr;
	ClientSessionInfo *info = session->GetSessionInfo();
	if(info != nullptr)
	{
		pl = info->GetPlayer();
		info->ClearResourceTransfer();
		session->ClearSessionInfo();
	}
	session->Drop(nwm::ClientDropped::Disconnected);
	if(pl != nullptr)
	{
		SGame *game = GetGameState();
		auto &ent = pl->GetEntity();
		NetPacket packet;
		packet->Write<unsigned int>(ent.GetIndex());
		game->RemoveEntity(&ent);
		BroadcastTCP("playerdisconnect",packet);
	}
}
WVLocalClient *ServerState::GetLocalClient() {return m_local.get();}
unsigned short ServerState::GetTCPPort() {return CUInt16(GetConVarInt("sv_port_tcp"));}
unsigned short ServerState::GetUDPPort() {return CUInt16(GetConVarInt("sv_port_udp"));}
bool ServerState::IsTCPOpen() const {return (IsServerRunning() && m_server->HasTCPConnection()) ? true : false;}
bool ServerState::IsUDPOpen() const {return (IsServerRunning() && m_server->HasUDPConnection()) ? true : false;}

void ServerState::HandleLuaNetPacket(WVServerClient *session,NetPacket &packet)
{
	if(!IsGameActive())
		return;
	SGame *game = static_cast<SGame*>(GetGameState());
	game->HandleLuaNetPacket(session,packet);
}

void ServerState::BroadcastTCP(unsigned int ID,NetPacket &packet)
{
	packet.SetMessageID(ID);
	if(m_local != nullptr)
		m_local->SendPacket(packet);
	if(!IsServerRunning())
		return;
	m_server->BroadcastPacket(nwm::Protocol::TCP,packet);
}
void ServerState::BroadcastUDP(unsigned int ID,NetPacket &packet)
{
	packet.SetMessageID(ID);
	if(m_local != nullptr)
		m_local->SendPacket(packet);
	if(!IsServerRunning())
		return;
	m_server->BroadcastPacket(nwm::Protocol::UDP,packet);
}
void ServerState::SendPacketTCP(unsigned int ID,NetPacket &packet,const nwm::RecipientFilter &rp)
{
	packet.SetMessageID(ID);
	CallCallbacks<void,std::reference_wrapper<NetPacket>,std::reference_wrapper<const nwm::RecipientFilter>>("OnSendPacketTCP",std::ref(packet),std::ref(rp));
	if(!IsServerRunning())
	{
		if(rp.GetRecipientCount() > 0 && rp.GetFilterType() != nwm::RecipientFilter::Type::Exclude && m_local != nullptr)
			m_local->SendPacket(packet,nwm::Protocol::TCP);
		return;
	}
	m_server->SendPacket(nwm::Protocol::TCP,packet,rp);
}
void ServerState::SendPacketUDP(unsigned int ID,NetPacket &packet,const nwm::RecipientFilter &rp)
{
	packet.SetMessageID(ID);
	CallCallbacks<void,std::reference_wrapper<NetPacket>,std::reference_wrapper<const nwm::RecipientFilter>>("OnSendPacketUDP",std::ref(packet),std::ref(rp));
	if(!IsServerRunning())
	{
		auto rpCount = rp.GetRecipientCount();
		if(((rpCount > 0 && rp.GetFilterType() != nwm::RecipientFilter::Type::Exclude) || (rpCount == 0 && rp.GetFilterType() == nwm::RecipientFilter::Type::Exclude)) && m_local != nullptr)
			m_local->SendPacket(packet,nwm::Protocol::UDP);
		return;
	}
	m_server->SendPacket(nwm::Protocol::UDP,packet,rp);
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

void ServerState::BroadcastTCP(const std::string &name,NetPacket &packet)
{
	unsigned int ID = GetClientMessageID(name);
	if(check_message_id(ID,name) == false)
		return;
	BroadcastTCP(ID,packet);
}
void ServerState::BroadcastUDP(const std::string &name,NetPacket &packet)
{
	unsigned int ID = GetClientMessageID(name);
	if(check_message_id(ID,name) == false)
		return;
	BroadcastUDP(ID,packet);
}
void ServerState::SendPacketTCP(const std::string &name,NetPacket &packet,const nwm::RecipientFilter &rp)
{
	unsigned int ID = GetClientMessageID(name);
	if(check_message_id(ID,name) == false)
		return;
	SendPacketTCP(ID,packet,rp);
}
void ServerState::SendPacketUDP(const std::string &name,NetPacket &packet,const nwm::RecipientFilter &rp)
{
	unsigned int ID = GetClientMessageID(name);
	if(check_message_id(ID,name) == false)
		return;
	SendPacketUDP(ID,packet,rp);
}
void ServerState::SendPacketTCP(const std::string &name,NetPacket &packet,WVServerClient *client)
{
	if(client == nullptr)
		return;
	nwm::RecipientFilter rp;
	rp.Add(client);
	SendPacketTCP(name,packet,rp);
}
void ServerState::SendPacketUDP(const std::string &name,NetPacket &packet,WVServerClient *client)
{
	if(client == nullptr)
		return;
	nwm::RecipientFilter rp;
	rp.Add(client);
	SendPacketUDP(name,packet,rp);
}

void ServerState::BroadcastTCP(const std::string &name) {NetPacket p; BroadcastTCP(name,p);}
void ServerState::BroadcastUDP(const std::string &name) {NetPacket p; BroadcastUDP(name,p);}
void ServerState::SendPacketTCP(const std::string &name,const nwm::RecipientFilter &rp) {NetPacket p; SendPacketTCP(name,p,rp);}
void ServerState::SendPacketUDP(const std::string &name,const nwm::RecipientFilter &rp) {NetPacket p; SendPacketUDP(name,p,rp);}
void ServerState::SendPacketTCP(const std::string &name,WVServerClient *client) {NetPacket p; SendPacketTCP(name,p,client);}
void ServerState::SendPacketUDP(const std::string &name,WVServerClient *client) {NetPacket p; SendPacketUDP(name,p,client);}
