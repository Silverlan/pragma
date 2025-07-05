// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __WISERVERBROWSER_H__
#define __WISERVERBROWSER_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/wiframe.h"
#include "wmserverdata.h"
#include <pragma/networking/game_server_data.hpp>
#include <networkmanager/udp_handler/udp_message_dispatcher.h>

#include <pragma/networking/master_server_query_dispatcher.hpp>

namespace pragma::networking {
	class DLLCLIENT DefaultMasterServerQueryDispatcher : public IMasterServerQueryDispatcher {
	  public:
		virtual void DoQueryServers(const Filter &filter) override;
	  protected:
		DefaultMasterServerQueryDispatcher();
		virtual void DoCancelQuery() override;
		virtual void DoPoll() override;
		virtual void DoPingServer(uint32_t serverIdx) override;
		friend IMasterServerQueryDispatcher;
	  private:
		std::unique_ptr<UDPMessageDispatcher> m_dispatcher;
	};
};

class WITableRow;
class DLLCLIENT WIServerBrowser : public WIFrame {
  public:
	WIServerBrowser();
	virtual ~WIServerBrowser() override;
	virtual void Initialize() override;
	virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
	virtual void SetSize(int x, int y) override;
	void Refresh();
  protected:
	struct ServerData {
		pragma::networking::MasterServerQueryResult queryResult;
		WIHandle row = {};
	};
	std::vector<ServerData> m_servers;
	WIHandle m_hServerList;
	WIHandle m_hRefresh;
	WIHandle m_hConnect;
	bool m_bRefreshScheduled;
	std::unique_ptr<pragma::networking::IMasterServerQueryDispatcher, void (*)(pragma::networking::IMasterServerQueryDispatcher *)> m_msQueryDispatcher = {nullptr, [](pragma::networking::IMasterServerQueryDispatcher *) {}};
	void AddServer(const pragma::networking::MasterServerQueryResult &queryResult);
	void OnServerDoubleClick(unsigned int idx);
	void DisplayMessage(std::string msg);
	void DoRefresh();
};

#endif
