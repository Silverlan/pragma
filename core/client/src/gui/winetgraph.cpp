/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/gui/winetgraph.h"
#include <wgui/types/witext.h>
#include <pragma/networking/netmessages.h>

import pragma.string.unicode;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

static const uint32_t DATA_RECORD_BACKLOG = 30;

LINK_WGUI_TO_CLASS(WINetGraph, WINetGraph);

WINetGraph::NetData::NetData() : lastUpdate(0) { Reset(); }
void WINetGraph::NetData::Reset()
{
	if(client != nullptr)
		lastUpdate = client->RealTime();
	dataOutUDP = 0;
	dataOutTCP = 0;
	dataIn = 0;
	countOutUDP = 0;
	countOutTCP = 0;
	countIn = 0;
	messages.clear();
}

WINetGraph::NetData::MessageInfo::MessageInfo(uint64_t _size) : size(_size), count(1) {}

//////////////////////////////

WINetGraph::WINetGraph() : WIBase(), m_graphOffset(0), m_dataSizeIdx(0)
{
	m_netData.Reset();
	m_dataSizes.resize(DATA_RECORD_BACKLOG, 0);
}

WINetGraph::~WINetGraph()
{
	if(m_cbThink.IsValid())
		m_cbThink.Remove();
	if(m_cbOnPacketReceive.IsValid())
		m_cbOnPacketReceive.Remove();
	if(m_cbOnSendPacketUDP.IsValid())
		m_cbOnSendPacketUDP.Remove();
	if(m_cbOnSendPacketTCP.IsValid())
		m_cbOnSendPacketTCP.Remove();
}

void WINetGraph::AddGraphValue(uint32_t sz)
{
	if(!m_hPacketGraph.IsValid())
		return;
	auto *pLineGraph = m_hPacketGraph.get<WILineGraph>();
	pLineGraph->SetValue(m_graphOffset++, sz / static_cast<float>(1 << 10));
	if(m_graphOffset >= pLineGraph->GetSegmentCount())
		m_graphOffset = 0;
}

WIText *WINetGraph::CreateText(const std::string &text)
{
	auto &colText = Color::White;
	auto &colShadow = Color::Black;
	Vector2i shadowOffset {2.f, 2.f};

	auto *pText = WGUI::GetInstance().Create<WIText>(this);
	pText->SetColor(colText);
	pText->EnableShadow(true);
	pText->SetShadowColor(colShadow);
	pText->SetShadowOffset(shadowOffset);
	pText->SetText(text);
	pText->SizeToContents();
	return pText;
}

void WINetGraph::Initialize()
{
	WIBase::Initialize();

	auto *pPacketGraph = WGUI::GetInstance().Create<WILineGraph>(this);
	pPacketGraph->SetSize(256, 90);
	pPacketGraph->SetSegmentCount(100);
	m_hPacketGraph = pPacketGraph->GetHandle();

	auto *pDataGraph = WGUI::GetInstance().Create<WILineGraph>(this);
	pDataGraph->SetSize(pPacketGraph->GetSize());
	pDataGraph->SetSegmentCount(DATA_RECORD_BACKLOG);
	m_hDataGraph = pDataGraph->GetHandle();

	auto *pTextIncoming = CreateText("in:");
	m_txtIncoming = pTextIncoming->GetHandle();

	auto *pTextOutgoing = CreateText("out:");
	m_txtOutgoing = pTextOutgoing->GetHandle();

	std::stringstream ssUpdateRate;
	ssUpdateRate << "update rate: " << client->GetConVarInt("cl_updaterate");

	auto *pTextUpdateRate = CreateText(ssUpdateRate.str());
	m_txtUpdateRate = pTextUpdateRate->GetHandle();

	std::stringstream ssTickRate;
	ssTickRate << "tick rate: " << client->GetConVarInt("sv_tickrate");

	auto *pTextTickRate = CreateText(ssTickRate.str());
	m_txtTickRate = pTextTickRate->GetHandle();

	auto *pLatency = CreateText("latency: 000ms");
	m_hLatency = pLatency->GetHandle();

	auto *pLostPackets = CreateText("lost packets: 000");
	m_hLostPackets = pLostPackets->GetHandle();

	uint8_t numMessagesLines = 4;
	m_txtMessages.reserve(numMessagesLines);
	for(uint8_t i = 0; i < numMessagesLines; ++i) {
		auto *pText = CreateText(std::string("#") + std::to_string(i));
		m_txtMessages.push_back(pText->GetHandle());
	}

	m_cbThink = client->AddCallback("Think", FunctionCallback<void>::Create([this]() {
		auto &t = client->RealTime();
		auto tDelta = t - m_netData.lastUpdate;
		if(tDelta >= 1.0) {
			m_dataSizes[m_dataSizeIdx++] = m_netData.dataIn;
			if(m_dataSizeIdx >= m_dataSizes.size())
				m_dataSizeIdx = 0;
			if(m_txtIncoming.IsValid()) {
				std::stringstream ss;
				ss << "in: " << util::get_pretty_bytes(m_netData.dataIn) << "/s (" << m_netData.countIn << " packets)";

				auto *pText = m_txtIncoming.get<WIText>();
				pText->SetText(ss.str());
				pText->SizeToContents();
			}
			if(m_txtOutgoing.IsValid()) {
				std::stringstream ss;
				ss << "out: " << util::get_pretty_bytes(m_netData.dataOutUDP + m_netData.dataOutTCP) << "/s (" << (m_netData.countOutUDP + m_netData.countOutTCP) << " packets)";

				auto *pText = m_txtOutgoing.get<WIText>();
				pText->SetText(ss.str());
				pText->SizeToContents();
			}
			if(m_hLatency.IsValid()) {
				std::stringstream ss;
				ss << "latency: " << +c_game->GetLatency() << "ms";

				auto *pText = m_hLatency.get<WIText>();
				pText->SetText(ss.str());
				pText->SizeToContents();
			}
			if(m_hLostPackets.IsValid()) {
				std::stringstream ss;
				ss << "lost packets: " << c_game->GetLostPacketCount();

				auto *pText = m_hLostPackets.get<WIText>();
				pText->SetText(ss.str());
				pText->SizeToContents();
			}
			std::sort(m_netData.messages.begin(), m_netData.messages.end(), [](const std::pair<uint32_t, NetData::MessageInfo> &a, const std::pair<uint32_t, NetData::MessageInfo> &b) { return a.second.size > b.second.size; });
			auto *map = GetClientMessageMap();
			std::unordered_map<std::string, unsigned int> *netmessages;
			map->GetNetMessages(&netmessages);
			uint32_t idx = 0;
			for(auto &p : m_netData.messages) {
				auto msgId = p.first;
				auto it = std::find_if(netmessages->begin(), netmessages->end(), [msgId](const std::pair<std::string, unsigned int> &p) { return (p.second == msgId) ? true : false; });
				std::string name = (it != netmessages->end()) ? it->first : "Unknown";
				auto &info = p.second;
				auto &hLine = m_txtMessages[idx];
				if(hLine.IsValid()) {
					auto *pText = hLine.get<WIText>();
					pText->SetVisible(true);
					std::stringstream ss;
					ss << "#" << (idx + 1) << ": " << name << ": " << util::get_pretty_bytes(info.size) << " (x" << info.count << ")";
					pText->SetText(ss.str());
					pText->SizeToContents();
				}

				if(idx >= m_txtMessages.size())
					break;
				++idx;
			}
			for(auto i = idx; i < m_txtMessages.size(); ++i) {
				auto &hLine = m_txtMessages[i];
				if(hLine.IsValid())
					hLine->SetVisible(false);
			}
			UpdateGraph();
			m_netData.Reset();
		}
	}));
	m_cbOnPacketReceive = client->AddCallback("OnReceivePacket", FunctionCallback<void, std::reference_wrapper<NetPacket>>::Create([this](std::reference_wrapper<NetPacket> packet) {
		auto packetSize = packet.get()->GetDataSize();
		auto msgId = packet.get().GetMessageID();
		auto it = std::find_if(m_netData.messages.begin(), m_netData.messages.end(), [msgId](const std::pair<uint32_t, NetData::MessageInfo> &p) { return (p.first == msgId) ? true : false; });
		if(it != m_netData.messages.end()) {
			it->second.size += packetSize;
			++it->second.count;
		}
		else
			m_netData.messages.push_back({msgId, {packetSize}});
		AddGraphValue(packetSize);
		m_netData.dataIn += packetSize;
		++m_netData.countIn;
	}));
	m_cbOnSendPacketTCP = client->AddCallback("OnSendPacketTCP", FunctionCallback<void, std::reference_wrapper<NetPacket>>::Create([this](std::reference_wrapper<NetPacket> packet) {
		m_netData.dataOutTCP += packet.get()->GetDataSize();
		++m_netData.countOutTCP;
	}));
	m_cbOnSendPacketUDP = client->AddCallback("OnSendPacketUDP", FunctionCallback<void, std::reference_wrapper<NetPacket>>::Create([this](std::reference_wrapper<NetPacket> packet) {
		m_netData.dataOutUDP += packet.get()->GetDataSize();
		++m_netData.countOutUDP;
	}));
}

void WINetGraph::UpdateGraph()
{
	if(m_hPacketGraph.IsValid())
		m_hPacketGraph.get<WILineGraph>()->Update();

	if(!m_hDataGraph.IsValid())
		return;
	auto *pDataGraph = m_hDataGraph.get<WILineGraph>();
	auto offset = m_dataSizeIdx;
	for(auto i = decltype(DATA_RECORD_BACKLOG) {0}; i < DATA_RECORD_BACKLOG; ++i) {
		auto sz = m_dataSizes[offset] / static_cast<float>(1 << 15);
		pDataGraph->SetValue(i, sz);
		++offset;
		if(offset >= DATA_RECORD_BACKLOG)
			offset = 0;
	}
	if(m_hDataGraph.IsValid())
		m_hDataGraph.get<WILineGraph>()->Update();
}

void WINetGraph::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);

	if(!m_hPacketGraph.IsValid())
		return;
	m_hPacketGraph->SetX(x - m_hPacketGraph->GetWidth());

	if(m_hDataGraph.IsValid())
		m_hDataGraph->SetPos(m_hPacketGraph->GetX(), m_hPacketGraph->GetY() + m_hPacketGraph->GetHeight());

	if(!m_txtIncoming.IsValid())
		return;
	int32_t margin = 10;
	m_txtIncoming->SetPos(margin, margin);

	if(!m_txtOutgoing.IsValid())
		return;
	m_txtOutgoing->SetPos(margin, m_txtIncoming->GetY() + m_txtIncoming->GetHeight());

	if(!m_txtUpdateRate.IsValid())
		return;
	m_txtUpdateRate->SetPos(margin, m_txtOutgoing->GetY() + m_txtOutgoing->GetHeight());

	if(m_hLatency.IsValid())
		m_hLatency->SetPos(m_hDataGraph->GetX() - m_hLatency->GetWidth(), m_txtUpdateRate->GetY());

	if(!m_txtTickRate.IsValid())
		return;
	m_txtTickRate->SetPos(margin, m_txtUpdateRate->GetY() + m_txtUpdateRate->GetHeight());

	if(m_hLostPackets.IsValid())
		m_hLostPackets->SetPos(m_hDataGraph->GetX() - m_hLostPackets->GetWidth(), m_txtTickRate->GetY());

	auto yOffset = m_txtTickRate->GetY() + m_txtTickRate->GetHeight();
	for(auto &hLine : m_txtMessages) {
		if(hLine.IsValid()) {
			hLine->SetPos(margin, yOffset);
			yOffset += hLine->GetHeight();
		}
	}
}
