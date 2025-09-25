// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <deque>
#include <mutex>
#include <networkmanager/nwm_endpoint.h>
#include <sharedutils/util_clock.hpp>
#include <sharedutils/netpacket.hpp>
#include <sharedutils/scope_guard.h>

export module pragma.shared:networking.message_tracker;

export namespace pragma {
	namespace networking {
		class DLLNETWORK MessageTracker {
		  public:
			struct MessageInfo {
				NWMEndpoint endpoint;
				NetPacket packet;
				util::Clock::time_point tp;
				uint32_t id = std::numeric_limits<uint32_t>::max();
			};
			enum class MessageType : uint8_t { Incoming = 0, Outgoing };

			virtual ~MessageTracker() = default;
			std::pair<const std::deque<MessageInfo> &, std::unique_ptr<const util::ScopeGuard>> GetTrackedMessages(MessageType mt) const;
			void SetMemoryCount(uint32_t count);

			void DebugPrint(const std::unordered_map<std::string, uint32_t> &inMsgs, const std::unordered_map<std::string, uint32_t> &outMsgs);
			void DebugDump(const std::string &dumpFileName, const std::unordered_map<std::string, uint32_t> &inMsgs, const std::unordered_map<std::string, uint32_t> &outMsgs);

			void MemorizeNetMessage(MessageType mt, uint32_t id, const NWMEndpoint &ep, const NetPacket &packet);
		  protected:
			MessageTracker();
			std::pair<std::deque<MessageInfo> &, std::unique_ptr<const util::ScopeGuard>> GetTrackedMessages(MessageType mt);
			std::deque<MessageInfo> m_trackedMessagesIn;
			std::deque<MessageInfo> m_trackedMessagesOut;
		  private:
			uint32_t m_memCount = 0;
			mutable std::mutex m_trackedMessageMutex;
		};
	};
};
