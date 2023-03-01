/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networking/nwm_message_tracker.hpp"
#include <sharedutils/scope_guard.h>
#include <sharedutils/util_date.hpp>

pragma::networking::MessageTracker::MessageTracker() { SetMemoryCount(10); }

std::pair<const std::deque<pragma::networking::MessageTracker::MessageInfo> &, std::unique_ptr<const util::ScopeGuard>> pragma::networking::MessageTracker::GetTrackedMessages(MessageType mt) const
{
	m_trackedMessageMutex.lock();
	return {(mt == MessageType::Incoming) ? m_trackedMessagesIn : m_trackedMessagesOut, std::make_unique<util::ScopeGuard>([this]() { m_trackedMessageMutex.unlock(); })};
}
std::pair<std::deque<pragma::networking::MessageTracker::MessageInfo> &, std::unique_ptr<const util::ScopeGuard>> pragma::networking::MessageTracker::GetTrackedMessages(MessageType mt)
{
	m_trackedMessageMutex.lock();
	return {(mt == MessageType::Incoming) ? m_trackedMessagesIn : m_trackedMessagesOut, std::make_unique<util::ScopeGuard>([this]() { m_trackedMessageMutex.unlock(); })};
}

void pragma::networking::MessageTracker::MemorizeNetMessage(MessageType mt, uint32_t id, const NWMEndpoint &ep, const NetPacket &packet)
{
	if(m_memCount == 0)
		return;
	auto pair = GetTrackedMessages(mt);
	auto &msgs = pair.first;
	if(msgs.size() >= m_memCount)
		msgs.pop_front();
	msgs.push_back({});

	auto &msgInfo = msgs.back();
	msgInfo.id = id;
	msgInfo.endpoint = ep;
	msgInfo.packet = packet;
	msgInfo.tp = util::Clock::now();
}

void pragma::networking::MessageTracker::SetMemoryCount(uint32_t count)
{
	m_memCount = count;
	m_trackedMessageMutex.lock();
	while(m_trackedMessagesIn.size() > count)
		m_trackedMessagesIn.pop_front();
	while(m_trackedMessagesOut.size() > count)
		m_trackedMessagesOut.pop_front();
	m_trackedMessageMutex.unlock();
}

void pragma::networking::MessageTracker::DebugDump(const std::string &dumpFileName, const std::unordered_map<std::string, uint32_t> &inMsgs, const std::unordered_map<std::string, uint32_t> &outMsgs)
{
	auto f = FileManager::OpenFile<VFilePtrReal>(dumpFileName.c_str(), "wb");
	if(f == nullptr)
		return;
	for(auto type : {MessageType::Incoming, MessageType::Outgoing}) {
		switch(type) {
		case MessageType::Incoming:
			f->WriteString("IN");
			break;
		default:
			f->WriteString("OUT");
			break;
		}
		auto pair = GetTrackedMessages(type);
		auto &msgs = pair.first;
		f->Write<uint32_t>(msgs.size());
		for(auto &msg : msgs) {
			auto &regMsgs = (type == MessageType::Incoming) ? inMsgs : outMsgs;
			auto it = std::find_if(regMsgs.begin(), regMsgs.end(), [&msg](const std::pair<std::string, uint32_t> &nm) { return (msg.id == nm.second) ? true : false; });
			f->Write<uint32_t>(msg.id);
			auto msgName = (it != regMsgs.end()) ? it->first : "Unknown";
			f->WriteString(msgName);
			f->Write<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(util::clock::get_duration_since_start(msg.tp)).count());
			f->WriteString(msg.endpoint.GetIP());
			f->Write<uint64_t>(msg.packet->GetSize());
			f->Write(msg.packet->GetData(), msg.packet->GetSize());
		}
	}
}

void pragma::networking::MessageTracker::DebugPrint(const std::unordered_map<std::string, uint32_t> &inMsgs, const std::unordered_map<std::string, uint32_t> &outMsgs)
{
	auto tNow = util::Clock::now();
	for(auto type : {MessageType::Incoming, MessageType::Outgoing}) {
		auto pair = GetTrackedMessages(type);
		auto &msgs = pair.first;
		Con::cout << msgs.size() << " ";
		switch(type) {
		case MessageType::Incoming:
			Con::cout << "incoming ";
			break;
		default:
			Con::cout << "outgoing ";
			break;
		}
		Con::cout << "tracked messages found:" << Con::endl;
		for(auto &msg : msgs) {
			auto &regMsgs = (type == MessageType::Incoming) ? inMsgs : outMsgs;
			auto it = std::find_if(regMsgs.begin(), regMsgs.end(), [&msg](const std::pair<std::string, uint32_t> &nm) { return (msg.id == nm.second) ? true : false; });
			auto msgName = (it != regMsgs.end()) ? it->first : "Unknown";

			auto time = msg.tp;
			auto daypoint = date::floor<date::days>(time);
			auto tod = date::make_time(time - daypoint);

			Con::cout << msgName << " (" << msg.id << ") to " << msg.endpoint.GetIP() << " (Packet Size: " << msg.packet.get()->GetSize() << ").";
			switch(type) {
			case MessageType::Incoming:
				Con::cout << " Received ";
				break;
			default:
				Con::cout << " Sent ";
				break;
			}
			Con::cout << "at " << tod.hours().count() << ":" << tod.minutes().count() << ":" << tod.seconds().count() << "." << std::chrono::duration_cast<std::chrono::milliseconds>(tod.subseconds()).count() << "!" << Con::endl;
		}
	}
}
