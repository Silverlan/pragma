/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/animation2.hpp"
#include "pragma/model/animation/animated_pose.hpp"
#include "pragma/model/animation/animation_channel.hpp"
#include "pragma/model/animation/skeletal_animation.hpp"
#include "pragma/model/animation/activities.h"
#include "pragma/model/model.h"
#include <udm.hpp>
#include <mathutil/umath.h>
#pragma optimize("",off)
pragma::animation::AnimationChannel *pragma::animation::Animation2::AddChannel(const util::Path &path,udm::Type valueType)
{
	auto *channel = FindChannel(path);
	if(channel)
		return (channel->targetPath == path && channel->GetValueType() == valueType) ? channel : nullptr;
	m_channels.push_back(std::make_shared<AnimationChannel>());
	channel = m_channels.back().get();
	channel->SetValueType(valueType);
	channel->targetPath = path;
	return channel;
}

void pragma::animation::Animation2::AddChannel(AnimationChannel &channel)
{
	auto it = std::find_if(m_channels.begin(),m_channels.end(),[&channel](const std::shared_ptr<AnimationChannel> &channelOther) {
		return channelOther->targetPath == channel.targetPath;
	});
	if(it != m_channels.end())
	{
		*it = channel.shared_from_this();
		return;
	}
	m_channels.push_back(channel.shared_from_this());
}

pragma::animation::AnimationChannel *pragma::animation::Animation2::FindChannel(const util::Path &path)
{
	auto it = std::find_if(m_channels.begin(),m_channels.end(),[&path](const std::shared_ptr<AnimationChannel> &channel) {
		return channel->targetPath == path;
	});
	if(it == m_channels.end())
		return nullptr;
	return it->get();
}

std::ostream &operator<<(std::ostream &out,const pragma::animation::Animation2 &o)
{
	out<<"Animation2";
	out<<"[Dur:"<<o.GetDuration()<<"]";
	out<<"[Channels:"<<o.GetChannelCount()<<"]";
	out<<"[AnimSpeedFactor:"<<o.GetAnimationSpeedFactor()<<"]";
	return out;
}
#pragma optimize("",on)
