// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.net_graph;

export import :gui.line_graph;

export namespace pragma::gui::types {
	class DLLCLIENT WINetGraph : public WIBase {
	  private:
		struct DLLCLIENT NetData {
			NetData();
			void Reset();
			double lastUpdate;
			uint64_t dataOutUDP;
			uint64_t dataOutTCP;
			uint64_t dataIn;
			uint32_t countOutUDP;
			uint32_t countOutTCP;
			uint32_t countIn;
			struct DLLCLIENT MessageInfo {
				MessageInfo(uint64_t size);
				uint64_t size;
				uint32_t count;
			};
			std::vector<std::pair<uint32_t, MessageInfo>> messages;
		} m_netData;

		std::vector<uint64_t> m_dataSizes; // Incoming data size for the last x seconds
		std::size_t m_dataSizeIdx;
		uint32_t m_graphOffset;

		WIHandle m_hPacketGraph;
		WIHandle m_hDataGraph;

		WIHandle m_txtIncoming;
		WIHandle m_txtOutgoing;
		WIHandle m_txtUpdateRate;
		WIHandle m_txtTickRate;
		std::vector<WIHandle> m_txtMessages;
		WIHandle m_hLatency;
		WIHandle m_hLostPackets;
		CallbackHandle m_cbThink;
		CallbackHandle m_cbOnPacketReceive;
		CallbackHandle m_cbOnSendPacketUDP;
		CallbackHandle m_cbOnSendPacketTCP;
		void UpdateGraph();
		void AddGraphValue(uint32_t sz);
		WIText *CreateText(const std::string &text);
	  public:
		WINetGraph();
		virtual ~WINetGraph() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
	};
};
