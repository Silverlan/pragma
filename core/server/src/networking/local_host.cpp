#include "stdafx_server.h"
#include "pragma/networking/local_server.hpp"
#include "pragma/networking/recipient_filter.hpp"

extern DLLENGINE Engine *engine;

bool pragma::networking::LocalServer::DoStart(Error &outErr) {return true;}
bool pragma::networking::LocalServer::PollEvents(Error &outErr) {return true;}
bool pragma::networking::LocalServer::DoShutdown(Error &outErr) {return true;}
bool pragma::networking::LocalServer::Heartbeat() {return true;}
void pragma::networking::LocalServer::SetTimeoutDuration(float duration) {}

///////////////////

pragma::networking::LocalServerClient::~LocalServerClient() {}
uint16_t pragma::networking::LocalServerClient::GetLatency() const {return 0;}
std::string pragma::networking::LocalServerClient::GetIdentifier() const {return "localhost";}
std::optional<std::string> pragma::networking::LocalServerClient::GetIP() const {return {};}
std::optional<pragma::networking::Port> pragma::networking::LocalServerClient::GetPort() const {return {};}
bool pragma::networking::LocalServerClient::IsListenServerHost() const {return true;}
bool pragma::networking::LocalServerClient::SendPacket(pragma::networking::Protocol protocol,NetPacket &packet,pragma::networking::Error &outErr)
{
	packet.SetTimeActivated(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
	packet->SetOffset(0);
	engine->HandleLocalHostPlayerClientPacket(packet);
	return true;
}
bool pragma::networking::LocalServerClient::Drop(DropReason reason,pragma::networking::Error &outErr)
{
	// Local player can't actually be dropped from server; Just pretend that we did
	return true;
}
