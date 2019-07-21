#include "pr_module.hpp"
#include <pr_steam_networking_shared.hpp>
#include <sharedutils/util_weak_handle.hpp>
#include <pragma/networking/iserver.hpp>
#include <pragma/networking/iserver_client.hpp>
#include <pragma/networking/error.hpp>
#include <pragma/networking/ip_address.hpp>
#include <mathutil/umath.h>
#include <iostream>
#include <array>
#include <string>

#pragma optimize("",off)
class SteamServer;
class SteamServerClient
	: public pragma::networking::IServerClient
{
public:
	SteamServerClient(SteamServer &sv,HSteamNetConnection con);
	virtual bool Drop(pragma::networking::DropReason reason,pragma::networking::Error &outErr) override;
	virtual uint16_t GetLatency() const override;
	virtual std::string GetIdentifier() const override;
	virtual std::optional<std::string> GetIP() const override;
	virtual std::optional<pragma::networking::Port> GetPort() const override;
	virtual bool IsListenServerHost() const override;
	virtual bool SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr) override;
private:
	ISteamNetworkingSockets &GetInterface() const;
	HSteamNetConnection m_hConnection = k_HSteamNetConnection_Invalid;
	SteamServer &m_server;
};

SteamServerClient::SteamServerClient(SteamServer &sv,HSteamNetConnection con)
	: m_hConnection{con},m_server{sv}
{}

bool SteamServerClient::Drop(pragma::networking::DropReason reason,pragma::networking::Error &outErr)
{
	if(m_hConnection == k_HSteamNetConnection_Invalid)
		return true;
	if(GetInterface().CloseConnection(m_hConnection,k_ESteamNetConnectionEnd_App_Min +umath::to_integral(reason),pragma::networking::drop_reason_to_string(reason).c_str(),true) == false)
	{
		outErr = {pragma::networking::ErrorCode::UnableToDropClient,"Unable to drop client!"};
		return false;
	}
	m_hConnection = k_HSteamNetConnection_Invalid;
	return true;
}
uint16_t SteamServerClient::GetLatency() const
{
	return 0; // TODO
}
std::string SteamServerClient::GetIdentifier() const
{
	std::array<char,4'096> conName;
	if(GetInterface().GetConnectionName(m_hConnection,conName.data(),conName.size()) == false)
		return "";
	return conName.data();
}
std::optional<std::string> SteamServerClient::GetIP() const
{
	SteamNetConnectionInfo_t info;
	if(GetInterface().GetConnectionInfo(m_hConnection,&info) == false)
		return {};
	auto &address = info.m_addrRemote;
	std::array<char,MAX_IP_CHAR_LENGTH> ip;
	address.ToString(ip.data(),ip.size(),true);
	return ip.data();
}
std::optional<pragma::networking::Port> SteamServerClient::GetPort() const {return {};}
bool SteamServerClient::IsListenServerHost() const
{
	return false; // TODO
}
bool SteamServerClient::SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr)
{
	auto sendFlags = get_send_flags(protocol);
	auto res = GetInterface().SendMessageToConnection(m_hConnection,packet->GetData(true),packet->GetInternalSize(),sendFlags);
	if(res != EResult::k_EResultOK)
	{
		outErr = {pragma::networking::ErrorCode::UnableToSendPacket,"",res};
		return false;
	}
	return true;
}

/////////////

class SteamServer
	: public pragma::networking::IServer,
	private ISteamNetworkingSocketsCallbacks
{
public:
	virtual bool Start(pragma::networking::Error &outErr) override;
	virtual bool DoShutdown(pragma::networking::Error &outErr) override;
	virtual bool Heartbeat() override;
	virtual std::optional<std::string> GetHostIP() const override;
	virtual std::optional<pragma::networking::Port> GetLocalTCPPort() const override;
	virtual std::optional<pragma::networking::Port> GetLocalUDPPort() const override;
	virtual bool PollEvents(pragma::networking::Error &outErr) override;
	virtual void SetTimeoutDuration(float duration) override;

	ISteamNetworkingSockets &GetInterface() const;

	// Steam callbacks
	virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info) override;

	static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType,const char *msg);
protected:
	bool PollMessages(pragma::networking::Error &outErr);
private:
	ISteamNetworkingSockets *m_pInterface = nullptr;
	HSteamListenSocket m_hListenSock = 0;
	SteamNetworkingMicroseconds m_logTimeZero = 0;
	std::unordered_map<HSteamNetConnection,SteamServerClient*> m_conHandleToClient = {};
	pragma::networking::Error m_statusError = {};
};

void SteamServer::DebugOutput(ESteamNetworkingSocketsDebugOutputType eType,const char *msg)
{
	std::cout<<"Debug Output: "<<msg<<std::endl;
}

bool SteamServer::Start(pragma::networking::Error &outErr)
{
	SteamDatagramErrMsg errMsg;
	if(GameNetworkingSockets_Init(nullptr,errMsg) == false)
	{
		outErr = {pragma::networking::ErrorCode::UnableToStartServer,errMsg};
		return false;
	}
	m_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
	SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg,DebugOutput);
	m_pInterface = SteamNetworkingSockets();

	SteamNetworkingIPAddr serverLocalAddr;
	serverLocalAddr.Clear();
	serverLocalAddr.m_port = 29150; // TODO
	m_hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr);
	if(m_hListenSock == k_HSteamListenSocket_Invalid)
	{
		outErr = {pragma::networking::ErrorCode::UnableToListenOnPort,"Failed to listen on port " +std::to_string(serverLocalAddr.m_port) +"!"};
		return false;
	}
	return true;
}
bool SteamServer::DoShutdown(pragma::networking::Error &outErr)
{
	GameNetworkingSockets_Kill();
	return true;
}
bool SteamServer::Heartbeat()
{
	return true; // TODO
}
std::optional<std::string> SteamServer::GetHostIP() const
{
	return {}; // TODO
}
std::optional<pragma::networking::Port> SteamServer::GetLocalTCPPort() const {return {};}
std::optional<pragma::networking::Port> SteamServer::GetLocalUDPPort() const {return {};}
bool SteamServer::PollMessages(pragma::networking::Error &outErr)
{
	ISteamNetworkingMessage *pIncomingMsg = nullptr;
	auto numMsgs = m_pInterface->ReceiveMessagesOnListenSocket(m_hListenSock,&pIncomingMsg,1);
	if(numMsgs < 0)
		return false;
	if(numMsgs == 0)
		return true;
	ScopeGuard sgMsg {[pIncomingMsg]() {
		pIncomingMsg->Release();
	}};
	auto itClient = m_conHandleToClient.find(pIncomingMsg->m_conn);
	if(itClient == m_conHandleToClient.end())
	{
		outErr = {pragma::networking::ErrorCode::InvalidClient,"Received packet for unknown client!"};
		return false;
	}
	// TODO: Read header, then body!
	//NetPacket packet {msgId,size};
	//HandlePacket(*itClient->second,packet);
	return true;
}
bool SteamServer::PollEvents(pragma::networking::Error &outErr)
{
	auto success = PollMessages(outErr);
	m_pInterface->RunCallbacks(this);
	return success;
}
void SteamServer::SetTimeoutDuration(float duration) {}

ISteamNetworkingSockets &SteamServer::GetInterface() const {return *m_pInterface;}

void SteamServer::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
{
	m_statusError = {};
	switch(pInfo->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_None:
		break;
	case k_ESteamNetworkingConnectionState_Connecting:
	{
		auto result = m_pInterface->AcceptConnection(pInfo->m_hConn);
		if(result != k_EResultOK)
		{
			m_pInterface->CloseConnection(pInfo->m_hConn,0,nullptr,false);
			m_statusError = {pragma::networking::ErrorCode::UnableToAcceptClient,"Client could not be accepted!",result};
			break;
		}
		auto itClient = m_conHandleToClient.find(pInfo->m_hConn);
		if(itClient == m_conHandleToClient.end())
		{
			// New client
			auto cl = AddClient<SteamServerClient>(*this,pInfo->m_hConn);
			m_conHandleToClient[pInfo->m_hConn] = cl.get();
		}
		break;
	}
	case k_ESteamNetworkingConnectionState_FindingRoute:
		break;
	case k_ESteamNetworkingConnectionState_Connected:
		break;
	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
	{
		m_pInterface->CloseConnection(pInfo->m_hConn,0,nullptr,false);
		auto itCl = m_conHandleToClient.find(pInfo->m_hConn);
		if(itCl == m_conHandleToClient.end())
		{
			m_statusError = {pragma::networking::ErrorCode::InvalidClient,"Connection status of invalid client has changed!"};
			break;
		}
		m_conHandleToClient.erase(itCl);
		pragma::networking::Error err;
		DropClient(
			*itCl->second,
			(pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer) ? pragma::networking::DropReason::Disconnected : pragma::networking::DropReason::Error,
			err
		);
		break;
	}
	case k_ESteamNetworkingConnectionState_FinWait:
		break;
	case k_ESteamNetworkingConnectionState_Linger:
		break;
	case k_ESteamNetworkingConnectionState_Dead:
		break;
	}
}


ISteamNetworkingSockets &SteamServerClient::GetInterface() const
{
	return m_server.GetInterface();
}

class NetworkState;
extern "C"
{
	PRAGMA_EXPORT void initialize_game_server(NetworkState &nw,std::unique_ptr<pragma::networking::IServer> &outServer)
	{
		outServer = std::make_unique<SteamServer>();
	}
};
#pragma optimize("",on)
