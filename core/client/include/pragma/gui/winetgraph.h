/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WINETGRAPH_H__
#define __WINETGRAPH_H__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>
#include "pragma/gui/wilinegraph.h"

class WIText;
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

#endif
