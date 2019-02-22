#include "stdafx_client.h"
#include "pragma/clientstate/clientutil.h"
#include "pragma/gui/wiserverbrowser.h"
#include "pragma/gui/witable.h"
#include <wgui/types/wibutton.h>
#include "pragma/gui/wisilkicon.h"
#include "pragma/gui/wiscrollcontainer.h"
#include "pragma/localization.h"
#include "wms_shared.h"
#include "wms_request_filter.h"
#include "wms_message.h"
#include "pragma/networking/wv_message.h"

LINK_WGUI_TO_CLASS(WIServerBrowser,WIServerBrowser);

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
WIServerBrowser::WIServerBrowser()
	: WIFrame(),m_bRefreshScheduled(false),m_bDisconnectScheduled(false),
	m_batchCount(0)
{
	m_dispatcher = UDPMessageDispatcher::Create();
	if(m_dispatcher != nullptr)
		m_dispatcher->SetTimeout(4);
	AddStyleClass("wiframe");
	Refresh();
}

WIServerBrowser::~WIServerBrowser()
{}

void WIServerBrowser::ReceiveServerList(bool b)
{
	m_bDisconnectScheduled = true;
	if(b == false)
	{
		DisplayMessage(Locale::GetText("server_browser_connect_error"));
		return;
	}
	if(m_dispatchQueue.empty())
	{
		DisplayMessage(Locale::GetText("server_browser_no_servers"));
		return;
	}
	WITable *t = m_hServerList.get<WITable>();
	if(t != nullptr)
		t->Clear();
	DispatchBatch();
}

void WIServerBrowser::RemoveQueuedServer(WMServerData *info,bool addToList)
{
	auto it = std::find_if(m_waitQueue.begin(),m_waitQueue.end(),[info](std::unique_ptr<WMServerData> &pServerData) {
		return (info == pServerData.get()) ? true : false;
	});
	if(it == m_waitQueue.end())
	{
		DispatchBatch();
		return;
	}
	if(addToList == true)
		AddServer(*it);
	m_waitQueue.erase(it);
	DispatchBatch();
}

void WIServerBrowser::DispatchBatch()
{
	if(m_dispatcher == nullptr)
		return;
	auto numSend = umath::min(CUInt32(10),CUInt32(m_dispatchQueue.size())) -m_batchCount;
	if(numSend == 0)
		return;
	m_waitQueue.reserve(m_waitQueue.size() +numSend);
	for(auto it=m_dispatchQueue.begin();it!=m_dispatchQueue.end();)
	{
		m_waitQueue.push_back(std::move(*it));
		it = m_dispatchQueue.erase(it);
		auto &info = m_waitQueue.back();

		DataStream header;
		header->Write<uint16_t>(static_cast<uint16_t>(WVQuery::PING));
		header->Write<uint16_t>(static_cast<uint16_t>(0)); // Body Size

		auto *sv = info.get();
		m_dispatcher->Dispatch(header,info->ip,info->udpPort,[this,sv](boost::system::error_code err,UDPMessageDispatcher::Message *msg) mutable {
			if(!err)
			{
				msg->Receive(sizeof(WMSMessageHeader),[this,sv](boost::system::error_code err,DataStream data) {
					if(!err)
						RemoveQueuedServer(sv,true);
					else
					{
#ifdef _DEBUG
						Con::cwar<<"WARNING: Unable to add server '"<<sv->ip<<":"<<sv->tcpPort<<"' to list: "<<err.message()<<" ("<<err.value()<<")"<<Con::endl;
#endif
						RemoveQueuedServer(sv);
					}
				});
			}
			else
				RemoveQueuedServer(sv);
		});
	}
}

void WIServerBrowser::Think()
{
	if(m_dispatcher != nullptr)
		m_dispatcher->Poll();
	if(m_bRefreshScheduled == true)
		DoRefresh();
	WIFrame::Think();
}

void WIServerBrowser::OnServerDoubleClick(unsigned int idx)
{
	if(idx >= m_servers.size())
		return;
	auto &svInfo = m_servers[idx];
	auto &info = svInfo->info;
	c_engine->Connect(info->ip,std::to_string(info->tcpPort));
}

void WIServerBrowser::DisplayMessage(std::string msg)
{
	m_servers.clear();
	WITable *t = m_hServerList.get<WITable>();
	if(t == nullptr)
		return;
	t->Clear();
	WITableRow *row = t->AddRow();
	if(row == nullptr)
		return;
	row->SetValue(1,msg);
}

void WIServerBrowser::Initialize()
{
	WIFrame::Initialize();
	AddStyleClass("serverbrowser");
	SetTitle(Locale::GetText("server_browser"));
	m_hServerList = CreateChild<WITable>();
	WITable *t = m_hServerList.get<WITable>();
	if(t != nullptr)
	{
		t->SetSortable(true);
		t->SetScrollable(true);
		if(m_hTitleBar.IsValid())
			t->SetY(30);
		t->SetSelectable(true);
		WITableRow *row = t->AddHeaderRow();
		if(row != nullptr)
		{
			row->SetValue(0,Locale::GetText("password_protected"));
			row->SetValue(1,Locale::GetText("server_name"));
			row->SetValue(2,Locale::GetText("gamemode"));
			row->SetValue(3,Locale::GetText("map"));
			row->SetValue(4,Locale::GetText("players"));
			row->SetValue(5,Locale::GetText("latency"));
		}
		t->SetColumnWidth(0,30);
		t->SizeToContents();
	}
	m_hRefresh = CreateChild<WIButton>();
	WIButton *buttonRefresh = m_hRefresh.get<WIButton>();
	if(buttonRefresh != nullptr)
	{
		buttonRefresh->SetText(Locale::GetText("refresh"));
		buttonRefresh->AddCallback("OnPressed",FunctionCallback<>::Create(std::bind([](WIHandle hServerBrowser) {
			if(!hServerBrowser.IsValid())
				return;
			auto *sb = hServerBrowser.get<WIServerBrowser>();
			sb->Refresh();
		},this->GetHandle())));
	}
	m_hConnect = CreateChild<WIButton>();
	WIButton *buttonConnect = m_hConnect.get<WIButton>();
	if(buttonConnect != nullptr)
	{
		buttonConnect->SetText(Locale::GetText("connect"));
		buttonConnect->AddCallback("OnPressed",FunctionCallback<>::Create(std::bind([this](WIHandle hServerBrowser) {
			if(!hServerBrowser.IsValid())
				return;
			auto *sb = hServerBrowser.get<WIServerBrowser>();
			if(!sb->m_hServerList.IsValid())
				return;
			WITable *t = sb->m_hServerList.get<WITable>();
			if(t == nullptr)
				return;
			WITableRow *row = t->GetSelectedRow();
			if(row == nullptr)
				return;
			auto data = std::static_pointer_cast<int32_t>(row->GetUserData3());
			OnServerDoubleClick(*data);
		},this->GetHandle())));
	}
	SetSize(800,400);
	SetMinSize(400,300);
}

void WIServerBrowser::DoRefresh()
{
	m_bRefreshScheduled = false;
	if(m_dispatcher == nullptr)
		return;
	// TODO: What if something's still on m_dispatchQueue or m_waitQueue
	//if(m_hRefresh.IsValid())
	//	m_hRefresh.get<WIButton>()->Set
	DataStream body;

	auto filter = RequestFilter::NONE;
	//filter |= RequestFilter::NOT_EMPTY;
	body->Write<RequestFilter>(filter);
	std::unordered_map<std::string,std::string> filters;

	auto numFilters = std::min(static_cast<int>(filters.size()),50);
	body->Write<unsigned char>(CUChar(numFilters));
	for(auto it=filters.begin();it!=filters.end();++it)
	{
		body->WriteString(it->first);
		body->WriteString(it->second);
		numFilters--;
		if(numFilters == 0)
			break;
	}

	auto msgHeader = WMSMessageHeader(static_cast<unsigned int>(WMSMessage::REQUEST_SERVERS));
	msgHeader.size = CUInt16(body->GetSize());
	DataStream header;
	header->Write<WMSMessageHeader>(msgHeader);

	m_dispatcher->Dispatch(header,GetMasterServerIP(),GetMasterServerPort(),[this,body](boost::system::error_code err,UDPMessageDispatcher::Message*) mutable {
		if(!err)
		{
			m_dispatcher->Dispatch(body,GetMasterServerIP(),GetMasterServerPort(),[this](boost::system::error_code err,UDPMessageDispatcher::Message *msg) {
				if(!err)
				{
					msg->Receive(sizeof(WMSMessageHeader),[this,msg](boost::system::error_code err,DataStream data) {
						if(!err)
						{
							auto header = data->Read<WMSMessageHeader>();
							msg->Receive(header.size,[this](boost::system::error_code err,DataStream data) {
								if(!err)
								{
									auto numServers = data->Read<unsigned int>();
									m_dispatchQueue.reserve(numServers);
									for(unsigned int i=0;i<numServers;i++)
									{
										m_dispatchQueue.push_back(std::make_unique<WMServerData>());
										auto &info = m_dispatchQueue.back();
										info->ip = data->ReadString();
										WMServerData::Read(data,*info.get());
									}
									ReceiveServerList(true);
								}
								else
									ReceiveServerList(false);
							});
						}
						else
							ReceiveServerList(false);
					});
				}
				else
					ReceiveServerList(false);
			});
		}
		else
			ReceiveServerList(false);
	});
}

void WIServerBrowser::Refresh() {m_bRefreshScheduled = true;}

void WIServerBrowser::SetSize(int x,int y)
{
	WIFrame::SetSize(x,y);
	WITable *t = m_hServerList.get<WITable>();
	if(t != nullptr)
	{
		t->SetSize(x -20,y -100);
		t->SetX(10);
	}
	WIButton *buttonRefresh = m_hRefresh.get<WIButton>();
	if(buttonRefresh != nullptr)
	{
		buttonRefresh->SetWidth(100);
		buttonRefresh->SetPos(x -buttonRefresh->GetWidth() -10,y -buttonRefresh->GetHeight() -20);
	}
	WIButton *buttonConnect = m_hConnect.get<WIButton>();
	if(buttonConnect != nullptr)
	{
		buttonConnect->SetWidth(100);
		buttonConnect->SetPos(x -buttonConnect->GetWidth() -120,y -buttonConnect->GetHeight() -20);
	}
}

void WIServerBrowser::AddServer(std::unique_ptr<WMServerData> &data)
{
	if(!m_hServerList.IsValid())
		return;
	WITable *t = m_hServerList.get<WITable>();
	if(t == nullptr)
		return;
	WITableRow *row = t->AddRow();
	if(row != nullptr)
	{
		m_servers.push_back(std::unique_ptr<ServerInfo>(new ServerInfo(nullptr,data)));
		auto &info = m_servers.back();
		info->row = row->GetHandle();
		int idx = CInt32(m_servers.size() -1);
		row->SetUserData3(std::make_shared<int32_t>(idx));
		row->AddCallback("OnDoubleClick",FunctionCallback<>::Create(std::bind([this,idx](WIHandle hTableRow) {
			if(!hTableRow.IsValid())
				return;
			OnServerDoubleClick(idx);
		},row->GetHandle())));
		auto &data = info->info;
		if(data->password == true)
		{
			WISilkIcon *icon = WGUI::GetInstance().Create<WISilkIcon>();
			if(icon != nullptr)
			{
				icon->SetIcon("lock");
				row->InsertElement(0,icon);
			}
		}
		row->SetValue(1,data->name);
		row->SetValue(2,data->gameMode);
		row->SetValue(3,data->map);
		row->SetValue(4,std::to_string(data->players) +"/" +std::to_string(data->maxPlayers));
		row->SetValue(5,std::to_string(0));
	}
}

/////////////////////////////////////////

WIServerBrowser::ServerInfo::ServerInfo(WITableRow *_row,std::unique_ptr<WMServerData> &_info)
	: info(std::move(_info))
{
	if(_row != nullptr)
		row = _row->GetHandle();
}