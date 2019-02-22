#ifndef __WISERVERBROWSER_H__
#define __WISERVERBROWSER_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/wiframe.h"
#include "wmserverdata.h"
#include <networkmanager/udp_handler/udp_message_dispatcher.h>

class WITableRow;
class DLLCLIENT WIServerBrowser
	: public WIFrame
{
protected:
	class DLLCLIENT ServerInfo
	{
	public:
		ServerInfo(WITableRow *row,std::unique_ptr<WMServerData> &info);
		std::unique_ptr<WMServerData> info;
		WIHandle row;
	};
private:
	void RemoveQueuedServer(WMServerData *info,bool addToList=false);
protected:
	std::vector<std::unique_ptr<WMServerData>> m_dispatchQueue; // To be dispatched
	std::vector<std::unique_ptr<WMServerData>> m_waitQueue; // Waiting for response
	std::vector<std::unique_ptr<ServerInfo>> m_servers;
	unsigned int m_batchCount;
	WIHandle m_hServerList;
	WIHandle m_hRefresh;
	WIHandle m_hConnect;
	std::unique_ptr<UDPMessageDispatcher> m_dispatcher;
	bool m_bRefreshScheduled;
	bool m_bDisconnectScheduled;
	void AddServer(std::unique_ptr<WMServerData> &data);
	void ReceiveServerList(bool b);
	void OnServerDoubleClick(unsigned int idx);
	void DisplayMessage(std::string msg);
	void DoRefresh();
	void DispatchBatch();
public:
	WIServerBrowser();
	virtual ~WIServerBrowser() override;
	virtual void Initialize() override;
	virtual void Think() override;
	virtual void SetSize(int x,int y) override;
	void Refresh();
};

#endif