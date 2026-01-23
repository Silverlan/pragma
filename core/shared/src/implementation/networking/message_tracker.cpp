// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :console.output;
import :networking.message_tracker;

pragma::networking::MessageTracker::MessageTracker() { SetMemoryCount(10); }

std::pair<const std::deque<pragma::networking::MessageTracker::MessageInfo> &, std::unique_ptr<const pragma::util::ScopeGuard>> pragma::networking::MessageTracker::GetTrackedMessages(MessageType mt) const
{
	m_trackedMessageMutex.lock();
	return {(mt == MessageType::Incoming) ? m_trackedMessagesIn : m_trackedMessagesOut, std::make_unique<util::ScopeGuard>([this]() { m_trackedMessageMutex.unlock(); })};
}
std::pair<std::deque<pragma::networking::MessageTracker::MessageInfo> &, std::unique_ptr<const pragma::util::ScopeGuard>> pragma::networking::MessageTracker::GetTrackedMessages(MessageType mt)
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

void pragma::networking::MessageTracker::DebugDump(const std::string &dumpFileName, const util::StringMap<uint32_t> &inMsgs, const util::StringMap<uint32_t> &outMsgs)
{
	auto f = fs::open_file<fs::VFilePtrReal>(dumpFileName, fs::FileMode::Write | fs::FileMode::Binary);
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

void pragma::networking::MessageTracker::DebugPrint(const util::StringMap<uint32_t> &inMsgs, const util::StringMap<uint32_t> &outMsgs)
{
	auto tNow = util::Clock::now();
	for(auto type : {MessageType::Incoming, MessageType::Outgoing}) {
		auto pair = GetTrackedMessages(type);
		auto &msgs = pair.first;
		Con::COUT << msgs.size() << " ";
		switch(type) {
		case MessageType::Incoming:
			Con::COUT << "incoming ";
			break;
		default:
			Con::COUT << "outgoing ";
			break;
		}
		Con::COUT << "tracked messages found:" << Con::endl;
		for(auto &msg : msgs) {
			auto &regMsgs = (type == MessageType::Incoming) ? inMsgs : outMsgs;
			auto it = std::find_if(regMsgs.begin(), regMsgs.end(), [&msg](const std::pair<std::string, uint32_t> &nm) { return (msg.id == nm.second) ? true : false; });
			auto msgName = (it != regMsgs.end()) ? it->first : "Unknown";

			auto time = msg.tp;
			auto daypoint = date::floor<date::days>(time);
			auto tod = date::make_time(time - daypoint);

			Con::COUT << msgName << " (" << msg.id << ") to " << msg.endpoint.GetIP() << " (Packet Size: " << msg.packet.get()->GetSize() << ").";
			switch(type) {
			case MessageType::Incoming:
				Con::COUT << " Received ";
				break;
			default:
				Con::COUT << " Sent ";
				break;
			}
			Con::COUT << "at " << tod.hours().count() << ":" << tod.minutes().count() << ":" << tod.seconds().count() << "." << std::chrono::duration_cast<std::chrono::milliseconds>(tod.subseconds()).count() << "!" << Con::endl;
		}
	}
}
