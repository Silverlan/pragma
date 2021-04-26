/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __ANIMATED_POSE_HPP__
#define __ANIMATED_POSE_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/types.hpp"
#include <mathutil/transform.hpp>

class Skeleton;
namespace pragma::animation
{
	class DLLNETWORK AnimatedPose
	{
	public:
		AnimatedPose()=default;
		AnimatedPose(const AnimatedPose&)=default;
		AnimatedPose(AnimatedPose&&)=default;
		AnimatedPose(const std::vector<umath::ScaledTransform> &transforms);
		AnimatedPose(std::vector<umath::ScaledTransform> &&transforms);
		AnimatedPose &operator=(const AnimatedPose&)=default;
		AnimatedPose &operator=(AnimatedPose&&)=default;
		bool operator==(const AnimatedPose &other) const {return m_transforms == other.m_transforms;}
		bool operator!=(const AnimatedPose &other) const {return !operator==(other);}
		const std::vector<umath::ScaledTransform> &GetTransforms() const {return const_cast<AnimatedPose*>(this)->GetTransforms();}
		std::vector<umath::ScaledTransform> &GetTransforms() {return m_transforms;}
		void SetTransformCount(uint32_t c);
		void SetBoneIndex(uint32_t channelId,BoneId boneId);
		umath::ScaledTransform *GetTransform(BoneId idx);
		const umath::ScaledTransform *GetTransform(BoneId idx) const {return const_cast<AnimatedPose*>(this)->GetTransform(idx);}
		void SetTransform(BoneId idx,const umath::ScaledTransform &pose);
		void Clear();
		void Lerp(const AnimatedPose &other,float f);
		operator bool() const {return !m_transforms.empty();}

		std::vector<uint32_t> &GetBoneTranslationTable() {return m_boneIdToChannelId;}
		const std::vector<uint32_t> &GetBoneTranslationTable() const {return const_cast<AnimatedPose*>(this)->GetBoneTranslationTable();}

		void Localize(const Skeleton &skeleton);
		void Globalize(const Skeleton &skeleton);
	private:
		uint32_t GetChannelIdx(BoneId boneId) const;
		std::vector<umath::ScaledTransform> m_transforms {};
		std::vector<uint32_t> m_boneIdToChannelId;
		Skeleton *m_skeleton;
	};
};

#endif
