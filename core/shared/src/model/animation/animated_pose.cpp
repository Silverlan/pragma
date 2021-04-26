/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/animated_pose.hpp"
#include "pragma/model/animation/skeleton.h"

#pragma optimize("",off)
pragma::animation::AnimatedPose::AnimatedPose(const std::vector<umath::ScaledTransform> &transforms)
	: m_transforms{transforms}
{}
pragma::animation::AnimatedPose::AnimatedPose(std::vector<umath::ScaledTransform> &&transforms)
	: m_transforms{std::move(transforms)}
{}
void pragma::animation::AnimatedPose::SetTransformCount(uint32_t c) {m_transforms.resize(c);}
void pragma::animation::AnimatedPose::SetBoneIndex(uint32_t channelId,BoneId boneId)
{
	if(boneId >= m_boneIdToChannelId.size())
		m_boneIdToChannelId.resize(boneId +1,std::numeric_limits<uint32_t>::max());
	m_boneIdToChannelId[boneId] = channelId;
}
umath::ScaledTransform *pragma::animation::AnimatedPose::GetTransform(BoneId idx)
{
	if(idx >= m_boneIdToChannelId.size())
		return nullptr;
	auto channelIdx = m_boneIdToChannelId[idx];
	if(channelIdx >= m_transforms.size())
		return nullptr;
	return &m_transforms[channelIdx];
}
void pragma::animation::AnimatedPose::SetTransform(BoneId idx,const umath::ScaledTransform &pose)
{
	if(idx >= m_boneIdToChannelId.size())
		return;
	auto channelIdx = m_boneIdToChannelId[idx];
	if(channelIdx >= m_transforms.size())
		return;
	m_transforms[channelIdx] = pose;
}
void pragma::animation::AnimatedPose::Clear()
{
	m_transforms.clear();
	m_boneIdToChannelId.clear();
}
uint32_t pragma::animation::AnimatedPose::GetChannelIdx(BoneId boneId) const
{
	return (boneId < m_boneIdToChannelId.size()) ? m_boneIdToChannelId[boneId] : std::numeric_limits<uint32_t>::max();
}
void pragma::animation::AnimatedPose::Lerp(const AnimatedPose &other,float f)
{
	for(auto boneId=decltype(m_boneIdToChannelId.size()){0u};boneId<m_boneIdToChannelId.size();++boneId)
	{
		auto channel0 = GetChannelIdx(boneId);
		auto channel1 = other.GetChannelIdx(boneId);
		if(channel0 == std::numeric_limits<uint32_t>::max() || channel1 == std::numeric_limits<uint32_t>::max())
			continue;
		m_transforms[channel0].Interpolate(other.m_transforms[channel1],f);
	}
}
static void get_global_bone_transforms(const Skeleton &skeleton,pragma::animation::AnimatedPose &frame)
{
	std::function<void(pragma::animation::AnimatedPose&,const std::unordered_map<uint32_t,std::shared_ptr<Bone>>&,const Vector3&,const Quat&)> fGetGlobalBoneTransforms;
	fGetGlobalBoneTransforms = [&fGetGlobalBoneTransforms](pragma::animation::AnimatedPose &frame,const std::unordered_map<uint32_t,std::shared_ptr<Bone>> &bones,const Vector3 &posParent,const Quat &rotParent) {
		for(auto &pair : bones)
		{
			auto &bone = pair.second;
			assert(bone->ID == pair.first);
			auto *pose = frame.GetTransform(pair.first);
			if(pose)
			{
				auto &pos = pose->GetOrigin();
				auto &rot = pose->GetRotation();
				uvec::rotate(&pos,rotParent);
				pos += posParent;
				rot = rotParent *rot;
			}

			fGetGlobalBoneTransforms(frame,bone->children,pose ? pose->GetOrigin() : Vector3{},pose ? pose->GetRotation() : uquat::identity());
		}
	};
	fGetGlobalBoneTransforms(frame,skeleton.GetRootBones(),{},uquat::identity());
}
static void get_local_bone_transforms(const Skeleton &skeleton,pragma::animation::AnimatedPose &frame)
{
	std::function<void(pragma::animation::AnimatedPose&,const std::unordered_map<uint32_t,std::shared_ptr<Bone>>&)> fGetLocalBoneTransforms;
	fGetLocalBoneTransforms = [&fGetLocalBoneTransforms](pragma::animation::AnimatedPose &frame,const std::unordered_map<uint32_t,std::shared_ptr<Bone>> &bones) {
		for(auto it=bones.begin();it!=bones.end();++it)
		{
			auto &bone = it->second;
			fGetLocalBoneTransforms(frame,bone->children);

			auto parent = bone->parent.lock();
			if(parent != nullptr)
			{
				auto idx = it->first;
				auto parentIdx = parent->ID;

				auto *pose = frame.GetTransform(idx);
				if(pose)
				{
					auto &pos = pose->GetOrigin();
					auto &rot = pose->GetRotation();
					auto *poseParent = frame.GetTransform(parentIdx);
					if(poseParent)
					{
						pos -= poseParent->GetOrigin();
						auto inv = uquat::get_inverse(poseParent->GetRotation());
						uvec::rotate(&pos,inv);
						rot = inv *rot;
					}
				}
			}
		}
	};
	fGetLocalBoneTransforms(frame,skeleton.GetRootBones());
}
void pragma::animation::AnimatedPose::Localize(const Skeleton &skeleton)
{
	get_local_bone_transforms(skeleton,*this);
}
void pragma::animation::AnimatedPose::Globalize(const Skeleton &skeleton)
{
	get_global_bone_transforms(skeleton,*this);
}
#pragma optimize("",on)
