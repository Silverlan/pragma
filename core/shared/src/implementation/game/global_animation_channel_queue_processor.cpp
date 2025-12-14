// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.global_animation_channel_queue_processor;

import panima;

#undef GetCurrentTime

using namespace pragma;

GlobalAnimationChannelQueueProcessor::GlobalAnimationChannelQueueProcessor() : m_threadPool {} {}
void GlobalAnimationChannelQueueProcessor::Reset()
{
	m_itemCompleteCount = 0;
	m_totalItemCount = 0;
}
bool GlobalAnimationChannelQueueProcessor::IsPending() const { return m_itemCompleteCount < m_totalItemCount; }
GlobalAnimationChannelQueueProcessor::ChannelRange GlobalAnimationChannelQueueProcessor::GetNextRange()
{
	auto ul = std::unique_lock<std::mutex> {m_channelQueueMutex};
	m_channelQueueVar.wait(ul, [this]() -> bool { return !m_channelQueue.empty(); });

	auto range = m_channelQueue.front();
	m_channelQueue.pop();
	ul.unlock();
	return range;
}

void GlobalAnimationChannelQueueProcessor::ApplyValues()
{
	while(IsPending()) {
		auto range = GetNextRange();
		for(auto i = range.start; i < range.indexAfterLast; ++i) {
			auto &channelCacheData = (*range.cacheData)[i];
			if(!channelCacheData.memberInfo)
				continue;
			auto &component = *channelCacheData.component;
			auto &memberInfo = *channelCacheData.memberInfo;
			if(!math::is_flag_set(channelCacheData.changed, AnimationChannelCacheData::State::Dirty | AnimationChannelCacheData::State::AlwaysDirty))
				continue;
			memberInfo.setterFunction(memberInfo, component, channelCacheData.data.data());
			math::set_flag(const_cast<AnimationChannelCacheData &>(channelCacheData).changed, AnimationChannelCacheData::State::Dirty, false);
		}
		m_itemCompleteCount += (range.indexAfterLast - range.start);
	}
}
void GlobalAnimationChannelQueueProcessor::Submit(AnimationManagerData &amData)
{
	constexpr uint32_t numPerBatch = 50;

	auto &animManager = *amData.animationManager;
	auto *anim = animManager.GetCurrentAnimation();
	if(!anim)
		return;
	auto &channelValueSubmitters = animManager.GetChannelValueSubmitters();
	auto &channels = anim->GetChannels();
	if(channels.size() != channelValueSubmitters.size())
		throw std::runtime_error {"Number of channels does not match number of channel value submitters!"};
	auto n = math::min(channelValueSubmitters.size(), channels.size());
	auto t = animManager->GetCurrentTime();

	auto numItems = n;
	auto numBlocks = numItems / numPerBatch;
	if((numItems % numPerBatch) != 0)
		++numBlocks;
	m_threadPool.submit_blocks<size_t>(
	  0, numItems,
	  [this, &channels, &channelValueSubmitters, &amData, &animManager, t](size_t start, size_t indexAfterLast) {
		  for(auto i = start; i < indexAfterLast; ++i) {
			  auto &submitter = channelValueSubmitters[i];
			  if(!submitter)
				  continue;
			  auto &channel = channels[i];
			  if(channel->GetTimeCount() == 0)
				  continue;
			  submitter(*channel, animManager->GetLastChannelTimestampIndex(i), t);
		  }
		  AddToQueue(amData.channelCache, start, indexAfterLast);
	  },
	  numBlocks);
	m_totalItemCount += numItems;
}
void GlobalAnimationChannelQueueProcessor::AddToQueue(const std::vector<AnimationChannelCacheData> &cacheData, size_t start, size_t indexAfterLast)
{
	auto ul = std::unique_lock<std::mutex> {m_channelQueueMutex};
	m_channelQueue.push({&cacheData, start, indexAfterLast});
	m_channelQueueVar.notify_one();
}
