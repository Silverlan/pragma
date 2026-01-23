// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.server_browser;

export import :gui.frame;
export import pragma.shared;

export {
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
}

export namespace pragma::gui::types {
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
			networking::MasterServerQueryResult queryResult;
			WIHandle row = {};
		};
		std::vector<ServerData> m_servers;
		WIHandle m_hServerList;
		WIHandle m_hRefresh;
		WIHandle m_hConnect;
		bool m_bRefreshScheduled;
		std::unique_ptr<networking::IMasterServerQueryDispatcher, void (*)(networking::IMasterServerQueryDispatcher *)> m_msQueryDispatcher = {nullptr, [](networking::IMasterServerQueryDispatcher *) {}};
		void AddServer(const networking::MasterServerQueryResult &queryResult);
		void OnServerDoubleClick(unsigned int idx);
		void DisplayMessage(std::string msg);
		void DoRefresh();
	};
};
