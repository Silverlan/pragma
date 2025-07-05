// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __MASTER_SERVER_QUERY_DISPATCHER_HPP__
#define __MASTER_SERVER_QUERY_DISPATCHER_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/networking/game_server_data.hpp"
#include <optional>
#include <string>
#include <functional>
#include <memory>
#include <queue>
#include <cinttypes>
#include <mathutil/umath.h>

namespace pragma::networking {
	struct DLLNETWORK MasterServerQueryResult {
		GameServerInfo serverInfo = {};
		std::string ip = "";
		uint32_t numPlayers = 0u;
	};
	struct DLLNETWORK IMasterServerQueryDispatcher {
	  public:
		template<class TMasterServerQueryDispatcher>
		static std::unique_ptr<IMasterServerQueryDispatcher, void (*)(IMasterServerQueryDispatcher *)> Create();
		struct DLLNETWORK Filter {
			Filter(const std::string &networkLayerIdentifier) : networkLayerIdentifier {networkLayerIdentifier} {}
			enum class Flags : uint32_t { None = 0u, Dedicated, Secure, NotFull, NotEmpty, Empty, PasswordProtected };
			std::optional<std::string> map;
			std::string networkLayerIdentifier;
			Flags flags = Flags::None;
		};
		struct DLLNETWORK EventCallbacks {
			std::function<void(const MasterServerQueryResult &, bool)> onServerPingResponse = nullptr;
			std::function<void(bool, uint32_t)> onQueryResponse = nullptr;
			std::function<void(uint32_t)> onRefreshComplete = nullptr;
		};
		virtual ~IMasterServerQueryDispatcher() = default;
		void QueryServers(const Filter &filter);
		void Poll();
		void CancelQuery();
		void PingServers();
		void SetPingBatchCount();
		void SetEventCallbacks(const EventCallbacks &eventCallbacks);
	  protected:
		virtual void DoQueryServers(const Filter &filter) = 0;
		virtual void DoCancelQuery() = 0;
		virtual void DoPoll() {};
		virtual void DoPingServer(uint32_t serverIdx) = 0;
		void PingServer(uint32_t serverIdx);
		void AddQueryResult(MasterServerQueryResult &&queryResult);
		void OnServerPingResponse(uint32_t serverIdx, bool pingSuccessful);
		void OnQueryResponse(bool successful);
		MasterServerQueryResult *GetQueryResult(uint32_t idx);
		void DispatchPingBatch();
		virtual void Release();
		EventCallbacks m_eventCallbacks = {};
		uint32_t m_batchCount = 10u;
		uint32_t m_numServersPinged = 0u;
		std::vector<std::optional<MasterServerQueryResult>> m_queryResults = {};
		// Queue of server indices to be pinged
		std::queue<uint32_t> m_serverPingQueue = {};
	  private:
		void OnRefreshComplete();
	};
	REGISTER_BASIC_BITWISE_OPERATORS(IMasterServerQueryDispatcher::Filter::Flags)
};

template<class TMasterServerQueryDispatcher>
std::unique_ptr<pragma::networking::IMasterServerQueryDispatcher, void (*)(pragma::networking::IMasterServerQueryDispatcher *)> pragma::networking::IMasterServerQueryDispatcher::Create()
{
	return std::unique_ptr<IMasterServerQueryDispatcher, void (*)(IMasterServerQueryDispatcher *)> {new TMasterServerQueryDispatcher {}, [](IMasterServerQueryDispatcher *p) {
		                                                                                                p->Release();
		                                                                                                delete p;
	                                                                                                }};
}

#endif
