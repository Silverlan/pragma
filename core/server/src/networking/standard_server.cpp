#include "stdafx_server.h"
#include "pragma/networking/standard_server.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/networking/error.hpp>
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <networkmanager/nwm_error_handle.h>

extern DLLSERVER ServerState *server;

static void pragma_recipient_filter_to_nwm(const pragma::networking::NWMActiveServer &sv,const pragma::networking::ClientRecipientFilter &inRf,nwm::RecipientFilter &outRf)
{
	outRf = {};
	for(auto &cl : sv.GetClients())
	{
		auto &prCl = sv.GetPragmaClient(*cl.get());
		if(inRf(prCl) == false)
			continue;
		outRf.Add(cl.get());
	}
}
bool pragma::networking::StandardServer::DoStart(Error &outErr)
{
	auto tcpPort = server->GetConVarInt("sv_port_tcp");
	auto udpPort = server->GetConVarInt("sv_port_udp");
	try
	{
		m_server = NWMActiveServer::Create(static_cast<uint16_t>(tcpPort),static_cast<uint16_t>(udpPort),nwm::ConnectionType::TCPUDP);
		m_server->SetServer(*this);
	}
	catch(const NWMException &e)
	{
		std::stringstream serr {};
		serr<<"Unable to start server with ports TCP = "<<tcpPort<<" and UDP = "<<udpPort<<": "<<e.what();
		outErr = {ErrorCode::UnableToStartServer,serr.str()};
		return false;
	}
	if(m_server == nullptr)
		outErr = {ErrorCode::UnknownError};
	return m_server != nullptr;
}
bool pragma::networking::StandardServer::DoShutdown(Error &outErr)
{
	if(m_server == nullptr)
		return true;
	try
	{
		m_server->Close();
	}
	catch(const NWMException &e)
	{
		outErr = {ErrorCode::GenericError,e.what()};
		return false;
	}
	m_server = nullptr;
	return true;
}
std::optional<std::string> pragma::networking::StandardServer::GetHostIP() const
{
	return m_server->GetLocalIP();
}
std::optional<pragma::networking::Port> pragma::networking::StandardServer::GetLocalTCPPort() const
{
	return m_server->GetLocalTCPPort();
}
std::optional<pragma::networking::Port> pragma::networking::StandardServer::GetLocalUDPPort() const
{
	return m_server->GetLocalUDPPort();
}
bool pragma::networking::StandardServer::PollEvents(Error &outErr)
{
	m_server->PollEvents();
	return true;
}
void pragma::networking::StandardServer::SetTimeoutDuration(float duration)
{
	m_server->SetTimeoutDuration(duration);
}
bool pragma::networking::StandardServer::Heartbeat()
{
	m_server->Heartbeat();
	return true;
}
