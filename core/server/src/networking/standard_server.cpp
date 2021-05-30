/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
bool pragma::networking::StandardServer::DoStart(Error &outErr,uint16_t port,bool useP2PIfAvailable)
{
	auto tcpPort = port;
	auto udpPort = port;
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
std::optional<pragma::networking::Port> pragma::networking::StandardServer::GetHostPort() const
{
	return m_server->GetLocalTCPPort();
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
