// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :networking.standard_server;

import :server_state;

static void pragma_recipient_filter_to_nwm(const pragma::networking::NWMActiveServer &sv, const pragma::networking::ClientRecipientFilter &inRf, nwm::RecipientFilter &outRf)
{
	outRf = {};
	for(auto &cl : sv.GetClients()) {
		auto &prCl = sv.GetPragmaClient(*cl.get());
		if(inRf(prCl) == false)
			continue;
		outRf.Add(cl.get());
	}
}
bool pragma::networking::StandardServer::DoStart(Error &outErr, uint16_t port, bool useP2PIfAvailable)
{
	auto tcpPort = port;
	auto udpPort = port;
	try {
		m_server = NWMActiveServer::Create(static_cast<uint16_t>(tcpPort), static_cast<uint16_t>(udpPort), nwm::ConnectionType::TCPUDP);
		m_server->SetServer(*this);
	}
	catch(const NWMException &e) {
		std::stringstream serr {};
		serr << "Unable to start server with ports TCP = " << tcpPort << " and UDP = " << udpPort << ": " << e.what();
		outErr = {ErrorCode::UnableToStartServer, serr.str()};
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
	try {
		m_server->Close();
	}
	catch(const NWMException &e) {
		outErr = {ErrorCode::GenericError, e.what()};
		return false;
	}
	m_server = nullptr;
	return true;
}
std::optional<std::string> pragma::networking::StandardServer::GetHostIP() const { return m_server->GetLocalIP(); }
std::optional<pragma::networking::Port> pragma::networking::StandardServer::GetHostPort() const { return m_server->GetLocalTCPPort(); }
bool pragma::networking::StandardServer::PollEvents(Error &outErr)
{
	m_server->PollEvents();
	return true;
}
void pragma::networking::StandardServer::SetTimeoutDuration(float duration) { m_server->SetTimeoutDuration(duration); }
bool pragma::networking::StandardServer::Heartbeat()
{
	m_server->Heartbeat();
	return true;
}
