#include "pr_module.hpp"
#include <sharedutils/util_weak_handle.hpp>
#include <pragma/networking/iserver.hpp>
#include <pragma/networking/iserver_client.hpp>
#include <pragma/networking/error.hpp>
#include <mathutil/umath.h>
#include <iostream>
#include <array>

#undef ENABLE_STEAM_SERVER_SUPPORT

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifdef ENABLE_STEAM_SERVER_SUPPORT
#include <steam/steam_api.h>
#endif
#include <string>

class SteamServerClient
	: public pragma::networking::IServerClient
{
public:
	SteamServerClient(HSteamNetConnection con);
	virtual bool Drop(pragma::networking::DropReason reason,pragma::networking::Error &outErr) override;
	virtual uint16_t GetLatency() const override;
	virtual std::string GetIdentifier() const override;
	virtual std::optional<nwm::IPAddress> GetIPAddress() const override;
	virtual std::optional<std::string> GetIP() const override;
	virtual std::optional<pragma::networking::Port> GetPort() const override;
	virtual bool IsListenServerHost() const override;
	virtual bool SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr) override;
private:
	ISteamNetworkingSockets &GetInterface() const;
	HSteamNetConnection m_hConnection = k_HSteamNetConnection_Invalid;
};

SteamServerClient::SteamServerClient(HSteamNetConnection con)
	: m_hConnection{con}
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
std::optional<nwm::IPAddress> SteamServerClient::GetIPAddress() const
{
	SteamNetConnectionInfo_t info;
	if(GetInterface().GetConnectionInfo(m_hConnection,&info) == false)
		return {};
	auto &address = info.m_addrRemote;
	return nwm::IPAddress{}; // TODO;
}
std::optional<std::string> SteamServerClient::GetIP() const {return {};}
std::optional<pragma::networking::Port> SteamServerClient::GetPort() const {return {};}
bool SteamServerClient::IsListenServerHost() const
{
	return false; // TODO
}
bool SteamServerClient::SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr)
{
	int sendFlags = k_nSteamNetworkingSend_NoNagle; // TODO: Allow caller to control nagle?
	switch(protocol)
	{
	case pragma::networking::Protocol::FastUnreliable:
		sendFlags |= k_nSteamNetworkingSend_Unreliable;
		break;
	case pragma::networking::Protocol::SlowReliable:
	default:
		sendFlags |= k_nSteamNetworkingSend_Reliable;
		break;
	}
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
	virtual util::WeakHandle<pragma::networking::IServerClient> OnClientConnected() override;

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
	return true;
}
bool SteamServer::DoShutdown(pragma::networking::Error &outErr)
{
	GameNetworkingSockets_Kill();
	return true;
}
bool SteamServer::Heartbeat()
{

}
std::optional<std::string> SteamServer::GetHostIP() const
{

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
util::WeakHandle<pragma::networking::IServerClient> SteamServer::OnClientConnected()
{

}

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
			auto cl = AddClient<SteamServerClient>(pInfo->m_hConn);
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
		DropClient(*itCl->second,(pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer) ? pragma::networking::DropReason::Disconnected : pragma::networking::DropReason::Error);
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

/////////////

#include <pragma/networking/iclient.hpp>
class SteamClient
	: public pragma::networking::IClient,
	private ISteamNetworkingSocketsCallbacks
{
public:
	virtual std::string GetIdentifier() const override;
	virtual bool Connect(const std::string &ip,pragma::networking::Port port,pragma::networking::Error &outErr) override;
	virtual bool Disconnect(pragma::networking::Error &outErr) override;
	virtual bool SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr) override;
	virtual bool IsRunning() const override;
	virtual bool IsDisconnected() const override;
	virtual bool PollEvents(pragma::networking::Error &outErr) override;
	virtual uint16_t GetLatency() const override;
	virtual void SetTimeoutDuration(float duration) override;
	virtual std::optional<nwm::IPAddress> GetIPAddress() const override;
	virtual std::optional<std::string> GetIP() const override;
	virtual std::optional<pragma::networking::Port> GetLocalTCPPort() const override;
	virtual std::optional<pragma::networking::Port> GetLocalUDPPort() const override;
protected:
	bool PollMessages(pragma::networking::Error &outErr);
private:
	ISteamNetworkingSockets *m_pInterface = nullptr;
	HSteamNetConnection m_hConnection = k_HSteamNetConnection_Invalid;
};

std::string SteamClient::GetIdentifier() const
{
	std::array<char,4'096> conName;
	if(m_pInterface->GetConnectionName(m_hConnection,conName.data(),conName.size()) == false)
		return "";
	return conName.data();
}
bool SteamClient::Connect(const std::string &ip,pragma::networking::Port port,pragma::networking::Error &outErr)
{
	SteamNetworkingIPAddr ipAddr {};
	if(ipAddr.ParseString((ip +':' +std::to_string(port)).c_str()) == false)
		return false;
	m_hConnection = m_pInterface->ConnectByIPAddress(ipAddr);
	return !!m_hConnection;
}
bool SteamClient::Disconnect(pragma::networking::Error &outErr)
{
	return m_pInterface->CloseConnection(m_hConnection,reason,debugString,enableLinger);
}
bool SteamClient::SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr)
{
	m_pInterface->SendMessageToConnection(m_hConnection,data,dataSize,sendFlags);
}
bool SteamClient::IsRunning() const
{
	// TODO
}
bool SteamClient::IsDisconnected() const
{
	// TODO
}
bool SteamClient::PollMessages(pragma::networking::Error &outErr)
{
	ISteamNetworkingMessage *pIncomingMsg = nullptr;
	auto numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection,&pIncomingMsg,1);
	if(numMsgs < 0)
		return false;
	if(numMsgs == 0)
		return true;
	ScopeGuard sgMsg {[pIncomingMsg]() {
		pIncomingMsg->Release();
	}};
	//fwrite( pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout );
	// Just echo anything we get from the server
	
}
bool SteamClient::PollEvents(pragma::networking::Error &outErr)
{
	auto success = PollMessages(outErr);
	m_pInterface->RunCallbacks(this);
	return success;
}
uint16_t SteamClient::GetLatency() const
{
	return 0; // TODO
}
void SteamClient::SetTimeoutDuration(float duration) {}
std::optional<nwm::IPAddress> SteamClient::GetIPAddress() const
{
	SteamNetConnectionInfo_t info;
	if(m_pInterface->GetConnectionInfo(m_hConnection,&info) == false)
		return {};
	auto &address = info.m_addrRemote;
	return nwm::IPAddress{}; // TODO;
}
std::optional<std::string> SteamClient::GetIP() const {return {};}
std::optional<pragma::networking::Port> SteamClient::GetLocalTCPPort() const {return {};}
std::optional<pragma::networking::Port> SteamClient::GetLocalUDPPort() const {return {};}
