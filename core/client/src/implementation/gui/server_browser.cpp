// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.frame;
import :gui.scroll_container;
import :gui.server_browser;
import :gui.silk_icon;
import :gui.table;

import :client_state;
import :engine;
import pragma.gui;
import pragma.wms;

pragma::networking::DefaultMasterServerQueryDispatcher::DefaultMasterServerQueryDispatcher() : IMasterServerQueryDispatcher {}
{
	m_dispatcher = UDPMessageDispatcher::Create();
	if(m_dispatcher != nullptr)
		m_dispatcher->SetTimeout(4);
}

void pragma::networking::DefaultMasterServerQueryDispatcher::DoPoll()
{
	if(m_dispatcher != nullptr)
		m_dispatcher->Poll();
}

void pragma::networking::DefaultMasterServerQueryDispatcher::DoPingServer(uint32_t serverIdx)
{
	auto *queryResult = GetQueryResult(serverIdx);
	if(queryResult == nullptr)
		return;
	util::DataStream header;
	header->Write<uint16_t>(static_cast<uint16_t>(ServerQuery::Ping));
	header->Write<uint16_t>(static_cast<uint16_t>(0)); // Body Size

	m_dispatcher->Dispatch(header, queryResult->ip, queryResult->serverInfo.port, [this, serverIdx](nwm::ErrorCode err, UDPMessageDispatcher::Message *msg) mutable {
		if(!err) {
			msg->Receive(sizeof(WMSMessageHeader), [this, serverIdx](nwm::ErrorCode err, util::DataStream data) {
				if(!err)
					OnServerPingResponse(serverIdx, true);
				else {
#ifdef _DEBUG
					Con::CWAR << "Unable to add server '" << sv->ip << ":" << sv->tcpPort << "' to list: " << err.message() << " (" << err.value() << ")" << Con::endl;
#endif
					OnServerPingResponse(serverIdx, false);
				}
			});
		}
		else
			OnServerPingResponse(serverIdx, false);
	});
}

void pragma::networking::DefaultMasterServerQueryDispatcher::DoCancelQuery()
{
	// TODO
}

void pragma::networking::DefaultMasterServerQueryDispatcher::DoQueryServers(const Filter &filter)
{
	if(m_dispatcher == nullptr)
		return;
	// TODO: What if something's still on m_dispatchQueue or m_waitQueue
	//if(m_hRefresh.IsValid())
	//	m_hRefresh.get<WIButton>()->Set
	util::DataStream body;
	auto filterEnums = RequestFilter::AND;
	if(math::is_flag_set(filter.flags, Filter::Flags::NotEmpty))
		filterEnums |= RequestFilter::NOT_EMPTY;
	if(math::is_flag_set(filter.flags, Filter::Flags::NotFull))
		filterEnums |= RequestFilter::NOT_FULL;
	if(math::is_flag_set(filter.flags, Filter::Flags::Empty))
		filterEnums |= RequestFilter::EMPTY;
	if(math::is_flag_set(filter.flags, Filter::Flags::PasswordProtected) == false)
		filterEnums |= RequestFilter::NO_PASSWORD;
	body->Write<RequestFilter>(filterEnums);
	std::unordered_map<std::string, std::string> filters;

	auto numFilters = std::min(static_cast<int>(filters.size()), 50);
	body->Write<unsigned char>(CUChar(numFilters));
	for(auto it = filters.begin(); it != filters.end(); ++it) {
		body->WriteString(it->first);
		body->WriteString(it->second);
		numFilters--;
		if(numFilters == 0)
			break;
	}

	auto msgHeader = WMSMessageHeader(static_cast<unsigned int>(WMSMessage::REQUEST_SERVERS));
	msgHeader.size = CUInt16(body->GetSize());
	util::DataStream header;
	header->Write<WMSMessageHeader>(msgHeader);

	m_dispatcher->Dispatch(header, GetMasterServerIP(), GetMasterServerPort(), [this, body](nwm::ErrorCode err, UDPMessageDispatcher::Message *) mutable {
		if(!err) {
			m_dispatcher->Dispatch(body, GetMasterServerIP(), GetMasterServerPort(), [this](nwm::ErrorCode err, UDPMessageDispatcher::Message *msg) {
				if(!err) {
					msg->Receive(sizeof(WMSMessageHeader), [this, msg](nwm::ErrorCode err, util::DataStream data) {
						if(!err) {
							auto header = data->Read<WMSMessageHeader>();
							msg->Receive(header.size, [this](nwm::ErrorCode err, util::DataStream data) {
								if(!err) {
									auto numServers = data->Read<unsigned int>();
									for(unsigned int i = 0; i < numServers; i++) {
										WMServerData serverData {};
										serverData.ip = data->ReadString();
										WMServerData::Read(data, serverData);

										MasterServerQueryResult queryResult {};
										queryResult.ip = serverData.ip;
										queryResult.numPlayers = serverData.players;
										queryResult.serverInfo.botCount = serverData.bots;
										queryResult.serverInfo.gameMode = serverData.gameMode;
										queryResult.serverInfo.mapName = serverData.map;
										queryResult.serverInfo.maxPlayers = serverData.maxPlayers;
										queryResult.serverInfo.name = serverData.name;
										queryResult.serverInfo.passwordProtected = serverData.password;
										queryResult.serverInfo.port = serverData.tcpPort;
										queryResult.serverInfo.version = serverData.engineVersion;
										AddQueryResult(std::move(queryResult));
									}
									OnQueryResponse(true);
								}
								else
									OnQueryResponse(false);
							});
						}
						else
							OnQueryResponse(false);
					});
				}
				else
					OnQueryResponse(false);
			});
		}
		else
			OnQueryResponse(false);
	});
}

///////////////

pragma::gui::types::WIServerBrowser::WIServerBrowser() : WIFrame(), m_bRefreshScheduled(false)
{
	AddStyleClass("wiframe");
	Refresh();

	auto libSteamworks = get_client_state()->InitializeLibrary("steamworks/pr_steamworks");
	if(libSteamworks == nullptr)
		return;
	auto *fInitMasterServerQueryDispatcher = libSteamworks->FindSymbolAddress<void (*)(std::unique_ptr<networking::IMasterServerQueryDispatcher, void (*)(networking::IMasterServerQueryDispatcher *)> &)>("initialize_master_server_query_dispatcher");
	if(fInitMasterServerQueryDispatcher == nullptr)
		return;
	fInitMasterServerQueryDispatcher(m_msQueryDispatcher);
	if(fInitMasterServerQueryDispatcher == nullptr)
		return;
	networking::IMasterServerQueryDispatcher::EventCallbacks eventCallbacks {};
	eventCallbacks.onServerPingResponse = [this](const networking::MasterServerQueryResult &queryResult, bool pingSuccessful) {
		if(pingSuccessful)
			AddServer(queryResult);
	};
	eventCallbacks.onRefreshComplete = [this](uint32_t numServers) {
		if(numServers == 0u)
			DisplayMessage(locale::get_text("server_browser_no_servers"));
	};
	eventCallbacks.onQueryResponse = [this](bool querySuccessful, uint32_t numServersFound) {
		if(querySuccessful == false) {
			DisplayMessage(locale::get_text("server_browser_connect_error"));
			return;
		}
		if(numServersFound == 0u) {
			DisplayMessage(locale::get_text("server_browser_no_servers"));
			return;
		}
		WITable *t = m_hServerList.get<WITable>();
		if(t != nullptr)
			t->Clear();
		m_msQueryDispatcher->PingServers();
	};
	m_msQueryDispatcher->SetEventCallbacks(eventCallbacks);
}

pragma::gui::types::WIServerBrowser::~WIServerBrowser() { m_msQueryDispatcher = nullptr; }

void pragma::gui::types::WIServerBrowser::Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	if(m_msQueryDispatcher)
		m_msQueryDispatcher->Poll();
	if(m_bRefreshScheduled == true)
		DoRefresh();
	WIFrame::Think(drawCmd);
}

void pragma::gui::types::WIServerBrowser::OnServerDoubleClick(unsigned int idx)
{
	if(idx >= m_servers.size())
		return;
	auto &svInfo = m_servers.at(idx);
	get_client_state()->SetConVar("net_library", svInfo.queryResult.serverInfo.networkLayerIdentifier);
	if(svInfo.queryResult.serverInfo.peer2peer && svInfo.queryResult.serverInfo.steamId.has_value()) {
		auto steamId = *svInfo.queryResult.serverInfo.steamId;
		get_cengine()->Connect(steamId);
		return;
	}
	get_cengine()->Connect(svInfo.queryResult.ip, std::to_string(svInfo.queryResult.serverInfo.port));
}

void pragma::gui::types::WIServerBrowser::DisplayMessage(std::string msg)
{
	m_servers.clear();
	WITable *t = m_hServerList.get<WITable>();
	if(t == nullptr)
		return;
	t->Clear();
	WITableRow *row = t->AddRow();
	if(row == nullptr)
		return;
	row->SetValue(1, msg);
}

void pragma::gui::types::WIServerBrowser::Initialize()
{
	WIFrame::Initialize();
	EnableThinking();
	AddStyleClass("serverbrowser");
	SetTitle(locale::get_text("server_browser"));

	auto *contents = GetContents();
	if(!contents)
		return;
	auto &wgui = WGUI::GetInstance();
	m_hServerList = wgui.Create<WITable>(contents)->GetHandle();
	WITable *t = m_hServerList.get<WITable>();
	if(t != nullptr) {
		t->SetSortable(true);
		t->SetScrollable(true);
		t->SetSelectable(WITable::SelectableMode::Single);
		WITableRow *row = t->AddHeaderRow();
		if(row != nullptr) {
			row->SetValue(0, locale::get_text("password_protected"));
			row->SetValue(1, locale::get_text("server_name"));
			row->SetValue(2, locale::get_text("gamemode"));
			row->SetValue(3, locale::get_text("map"));
			row->SetValue(4, locale::get_text("players"));
			row->SetValue(5, locale::get_text("latency"));
		}
		t->SetColumnWidth(0, 30);
		t->SizeToContents();

		t->SetSize(contents->GetWidth() - 20, contents->GetHeight() - 100);
		t->SetX(10);
		t->SetAnchor(0.f, 0.f, 1.f, 1.f);
	}
	m_hRefresh = wgui.Create<WIButton>(contents)->GetHandle();
	WIButton *buttonRefresh = m_hRefresh.get<WIButton>();
	if(buttonRefresh != nullptr) {
		buttonRefresh->SetText(locale::get_text("refresh"));
		auto hServerBrowser = GetHandle();
		buttonRefresh->AddCallback("OnPressed", FunctionCallback<util::EventReply>::CreateWithOptionalReturn([hServerBrowser](util::EventReply *reply) mutable -> CallbackReturnType {
			*reply = util::EventReply::Handled;
			if(!hServerBrowser.IsValid())
				return CallbackReturnType::HasReturnValue;
			auto *sb = hServerBrowser.get<WIServerBrowser>();
			sb->Refresh();
			return CallbackReturnType::HasReturnValue;
		}));

		buttonRefresh->SetWidth(100);
		buttonRefresh->SetPos(contents->GetWidth() - buttonRefresh->GetWidth() - 10, contents->GetHeight() - buttonRefresh->GetHeight() - 20);
		buttonRefresh->SetAnchor(1.f, 1.f, 1.f, 1.f);
	}
	m_hConnect = wgui.Create<WIButton>(contents)->GetHandle();
	WIButton *buttonConnect = m_hConnect.get<WIButton>();
	if(buttonConnect != nullptr) {
		buttonConnect->SetText(locale::get_text("connect"));
		auto hServerBrowser = GetHandle();
		buttonConnect->AddCallback("OnPressed", FunctionCallback<util::EventReply>::CreateWithOptionalReturn([this, hServerBrowser](util::EventReply *reply) mutable -> CallbackReturnType {
			*reply = util::EventReply::Handled;
			if(!hServerBrowser.IsValid())
				return CallbackReturnType::HasReturnValue;
			auto *sb = hServerBrowser.get<WIServerBrowser>();
			if(!sb->m_hServerList.IsValid())
				return CallbackReturnType::HasReturnValue;
			auto *t = sb->m_hServerList.get<WITable>();
			if(t == nullptr)
				return CallbackReturnType::HasReturnValue;
			auto hRow = t->GetFirstSelectedRow();
			if(hRow.IsValid() == false)
				return CallbackReturnType::HasReturnValue;
			auto data = std::static_pointer_cast<int32_t>(hRow->GetUserData3());
			OnServerDoubleClick(*data);
			return CallbackReturnType::HasReturnValue;
		}));

		buttonConnect->SetWidth(100);
		buttonConnect->SetPos(contents->GetWidth() - buttonConnect->GetWidth() - 120, contents->GetHeight() - buttonConnect->GetHeight() - 20);
		buttonConnect->SetAnchor(1.f, 1.f, 1.f, 1.f);
	}
	SetSize(800, 400);
	SetMinSize(400, 300);
}

void pragma::gui::types::WIServerBrowser::DoRefresh()
{
	m_bRefreshScheduled = false;
	if(m_msQueryDispatcher) {
		networking::IMasterServerQueryDispatcher::Filter filter {get_client_state()->GetConVarString("networking_library")};
		m_msQueryDispatcher->QueryServers(filter);
	}
}

void pragma::gui::types::WIServerBrowser::Refresh() { m_bRefreshScheduled = true; }

void pragma::gui::types::WIServerBrowser::SetSize(int x, int y) { WIFrame::SetSize(x, y); }

void pragma::gui::types::WIServerBrowser::AddServer(const networking::MasterServerQueryResult &queryResult)
{
	if(!m_hServerList.IsValid())
		return;
	WITable *t = m_hServerList.get<WITable>();
	if(t == nullptr)
		return;
	WITableRow *row = t->AddRow();
	if(row != nullptr) {
		m_servers.push_back({});
		auto &data = m_servers.back();
		data.queryResult = queryResult;
		data.row = row->GetHandle();
		int idx = CInt32(m_servers.size() - 1);
		row->SetUserData3(pragma::util::make_shared<int32_t>(idx));
		auto hTableRow = row->GetHandle();
		row->AddCallback("OnDoubleClick", FunctionCallback<util::EventReply>::CreateWithOptionalReturn([this, hTableRow, idx](util::EventReply *reply) -> CallbackReturnType {
			*reply = util::EventReply::Handled;
			if(!hTableRow.IsValid())
				return CallbackReturnType::HasReturnValue;
			OnServerDoubleClick(idx);
			return CallbackReturnType::HasReturnValue;
		}));
		if(queryResult.serverInfo.passwordProtected == true) {
			WISilkIcon *icon = WGUI::GetInstance().Create<WISilkIcon>();
			if(icon != nullptr) {
				icon->SetIcon("lock");
				row->InsertElement(0, icon);
			}
		}
		row->SetValue(1, queryResult.serverInfo.name);
		row->SetValue(2, queryResult.serverInfo.gameMode);
		row->SetValue(3, queryResult.serverInfo.mapName);
		row->SetValue(4, std::to_string(queryResult.numPlayers) + "/" + std::to_string(queryResult.serverInfo.maxPlayers));
		row->SetValue(5, std::to_string(0));
	}
}
