/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __GLOBAL_ANIMATION_CHANNEL_QUEUE_HPP__
#define __GLOBAL_ANIMATION_CHANNEL_QUEUE_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/game/animation_channel_cache_data.hpp"
#include <sharedutils/BS_thread_pool.hpp>
#include <array>

import panima;

namespace pragma {
	class PanimaComponent;
	struct AnimationManagerData;
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
		void Submit(pragma::AnimationManagerData &amData);
		void ApplyValues();
		void Reset();

		BS::thread_pool &GetThreadPool() { return m_threadPool; }
	  private:
		void AddToQueue(const std::vector<AnimationChannelCacheData> &cacheData, size_t start, size_t indexAfterLast);
		BS::thread_pool m_threadPool;
		std::queue<ChannelRange> m_channelQueue;
		std::mutex m_channelQueueMutex;
		std::condition_variable m_channelQueueVar;
		size_t m_itemCompleteCount = 0;
		size_t m_totalItemCount = 0;
	};
};

#endif
