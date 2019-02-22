#ifndef __NWM_MESSAGE_TRACKER_HPP__
#define __NWM_MESSAGE_TRACKER_HPP__

#include "pragma/networkdefinitions.h"
#include <deque>
#include <chrono>
#include <mutex>
#include <networkmanager/nwm_endpoint.h>
#include <networkmanager/nwm_packet.h>
#include <sharedutils/scope_guard.h>

namespace pragma
{
	namespace networking
	{
		class DLLNETWORK MessageTracker
		{
		public:
			struct MessageInfo
			{
				NWMEndpoint endpoint;
				NetPacket packet;
				std::chrono::high_resolution_clock::time_point tp;
				uint32_t id = std::numeric_limits<uint32_t>::max();
			};
			enum class MessageType : uint8_t
			{
				Incoming = 0,
				Outgoing
			};

			virtual ~MessageTracker()=default;
			std::pair<const std::deque<MessageInfo>&,std::unique_ptr<const ScopeGuard>> GetTrackedMessages(MessageType mt) const;
			void SetMemoryCount(uint32_t count);

			void DebugPrint(const std::unordered_map<std::string,uint32_t> &inMsgs,const std::unordered_map<std::string,uint32_t> &outMsgs);
			void DebugDump(const std::string &dumpFileName,const std::unordered_map<std::string,uint32_t> &inMsgs,const std::unordered_map<std::string,uint32_t> &outMsgs);
		protected:
			MessageTracker();
			std::pair<std::deque<MessageInfo>&,std::unique_ptr<const ScopeGuard>> GetTrackedMessages(MessageType mt);
			void MemorizeNetMessage(MessageType mt,uint32_t id,const NWMEndpoint &ep,const NetPacket &packet);
			std::deque<MessageInfo> m_trackedMessagesIn;
			std::deque<MessageInfo> m_trackedMessagesOut;
		private:
			uint32_t m_memCount = 0;
			mutable std::mutex m_trackedMessageMutex;
		};
	};
};

#endif
