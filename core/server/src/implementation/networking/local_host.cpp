// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :networking.local_host;

bool pragma::networking::LocalServer::DoStart(Error &outErr, uint16_t port, bool useP2PIfAvailable) { return true; }
bool pragma::networking::LocalServer::PollEvents(Error &outErr) { return true; }
bool pragma::networking::LocalServer::DoShutdown(Error &outErr) { return true; }
bool pragma::networking::LocalServer::Heartbeat() { return true; }
void pragma::networking::LocalServer::SetTimeoutDuration(float duration) {}

///////////////////

pragma::networking::LocalServerClient::~LocalServerClient() {}
uint16_t pragma::networking::LocalServerClient::GetLatency() const { return 0; }
std::string pragma::networking::LocalServerClient::GetIdentifier() const { return "localhost"; }
std::optional<std::string> pragma::networking::LocalServerClient::GetIP() const { return {}; }
std::optional<pragma::networking::Port> pragma::networking::LocalServerClient::GetPort() const { return {}; }
bool pragma::networking::LocalServerClient::IsListenServerHost() const { return true; }
bool pragma::networking::LocalServerClient::SendPacket(Protocol protocol, NetPacket &packet, Error &outErr)
{
	packet.SetTimeActivated(util::clock::to_int(util::clock::get_duration_since_start()));
	packet->SetOffset(0);
	Engine::Get()->HandleLocalHostPlayerClientPacket(packet);
	return true;
}
bool pragma::networking::LocalServerClient::Drop(DropReason reason, Error &outErr)
{
	// Local player can't actually be dropped from server; Just pretend that we did
	return true;
}
