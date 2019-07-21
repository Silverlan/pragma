#include "pr_module.hpp"
#include <pr_steam_networking_shared.hpp>
#include <sharedutils/util_weak_handle.hpp>
#include <pragma/networking/iclient.hpp>
#include <pragma/networking/error.hpp>
#include <mathutil/umath.h>
#include <iostream>
#include <array>
#include <string>

#pragma optimize("",off)
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
	virtual std::optional<std::string> GetIP() const override;
	virtual std::optional<pragma::networking::Port> GetLocalTCPPort() const override;
	virtual std::optional<pragma::networking::Port> GetLocalUDPPort() const override;
	virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *cbInfo) override;
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
	// TODO
	return m_pInterface->CloseConnection(m_hConnection,k_ESteamNetConnectionEnd_App_Min,"",false);
}
bool SteamClient::SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr)
{
	auto sendFlags = get_send_flags(protocol);
	m_pInterface->SendMessageToConnection(m_hConnection,packet->GetData(true),packet->GetDataSize(),sendFlags);
	return true;
}
bool SteamClient::IsRunning() const
{
	// TODO
	return true;
}
bool SteamClient::IsDisconnected() const
{
	// TODO
	return false;
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
	return true;
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
std::optional<std::string> SteamClient::GetIP() const
{
	SteamNetConnectionInfo_t info;
	if(m_pInterface->GetConnectionInfo(m_hConnection,&info) == false)
		return {};
	auto &address = info.m_addrRemote;
	std::array<char,MAX_IP_CHAR_LENGTH> ip;
	address.ToString(ip.data(),ip.size(),true);
	return ip.data();
}
std::optional<pragma::networking::Port> SteamClient::GetLocalTCPPort() const {return {};}
std::optional<pragma::networking::Port> SteamClient::GetLocalUDPPort() const {return {};}

void SteamClient::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *cbInfo)
{
	// TODO
}

class NetworkState;
extern "C"
{
	PRAGMA_EXPORT void initialize_game_client(NetworkState &nw,std::unique_ptr<pragma::networking::IClient> &outClient)
	{
		outClient = std::make_unique<SteamClient>();
	}
};
#pragma optimize("",on)
