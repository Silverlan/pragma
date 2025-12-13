// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:game.global_animation_channel_queue_processor;

export import :entities.components.panima;
export import :game.animation_channel_cache_data;
import panima;

export namespace pragma {
	class DLLNETWORK GlobalAnimationChannelQueueProcessor {
	  public:
		struct DLLNETWORK ChannelRange {
			const std::vector<AnimationChannelCacheData> *cacheData;
			size_t start;
			size_t indexAfterLast;
		};
		GlobalAnimationChannelQueueProcessor();
		bool IsPending() const;
		ChannelRange GetNextRange();
		void Submit(AnimationManagerData &amData);
		void ApplyValues();
		void Reset();

		BS::light_thread_pool &GetThreadPool() { return m_threadPool; }
	  private:
		void AddToQueue(const std::vector<AnimationChannelCacheData> &cacheData, size_t start, size_t indexAfterLast);
		BS::light_thread_pool m_threadPool;
		std::queue<ChannelRange> m_channelQueue;
		std::mutex m_channelQueueMutex;
		std::condition_variable m_channelQueueVar;
		size_t m_itemCompleteCount = 0;
		size_t m_totalItemCount = 0;
	};
};
