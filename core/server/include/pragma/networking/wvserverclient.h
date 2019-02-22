#ifndef __WVSERVERCLIENT_H__
#define __WVSERVERCLIENT_H__

#include <servermanager/interface/sv_nwm_serverclient.hpp>
#include "pragma/networking/clientsessioninfo.h"
#include <memory>

namespace nwm {class Server;};
class WVServerClient
	: public nwm::ServerClient
{
public:
	friend nwm::Server;
protected:
	WVServerClient(nwm::Server *manager);
	virtual void OnClosed() override;
	uint8_t m_snapshotId = 0;
	std::unique_ptr<ClientSessionInfo> m_sessionInfo;
	std::vector<std::string> m_scheduledResources; // Scheduled resource files for download
public:
	virtual ~WVServerClient() override;
	ClientSessionInfo *GetSessionInfo();
	void ClearSessionInfo();
	uint8_t SwapSnapshotId();
	void Reset();
	void ScheduleResource(const std::string &fileName);
	std::vector<std::string> &GetScheduledResources();
};

#endif