// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_POSE_HPP__
#define __PRAGMA_POSE_HPP__

#include <mathutil/transform.hpp>

import panima;

namespace pragma::animation {
	class Pose {
	  public:
		Pose() = default;
		Pose(const Pose &) = default;
		Pose(Pose &&) = default;
		Pose(const std::vector<umath::ScaledTransform> &transforms);
		Pose(std::vector<umath::ScaledTransform> &&transforms);
		Pose &operator=(const Pose &) = default;
		Pose &operator=(Pose &&) = default;
		bool operator==(const Pose &other) const { return m_transforms == other.m_transforms; }
		bool operator!=(const Pose &other) const { return !operator==(other); }
		const std::vector<umath::ScaledTransform> &GetTransforms() const { return const_cast<Pose *>(this)->GetTransforms(); }
		std::vector<umath::ScaledTransform> &GetTransforms() { return m_transforms; }
		void SetTransformCount(uint32_t c);
		void SetBoneIndex(uint32_t channelId, BoneId boneId);
		umath::ScaledTransform *GetTransform(pragma::animation::BoneId idx);
		const umath::ScaledTransform *GetTransform(pragma::animation::BoneId idx) const { return const_cast<Pose *>(this)->GetTransform(idx); }
		void SetTransform(pragma::animation::BoneId idx, const umath::ScaledTransform &pose);
		void Clear();
		void Lerp(const Pose &other, float f);
		operator bool() const { return !m_transforms.empty(); }

		void Localize(const pragma::animation::Skeleton &skeleton);
		void Globalize(const pragma::animation::Skeleton &skeleton);

		std::vector<uint32_t> &GetBoneTranslationTable() { return m_boneIdToChannelId; }
		const std::vector<uint32_t> &GetBoneTranslationTable() const { return const_cast<Pose *>(this)->GetBoneTranslationTable(); }
	  private:
		uint32_t GetChannelIdx(pragma::animation::BoneId boneId) const;
		std::vector<umath::ScaledTransform> m_transforms {};
		std::vector<uint32_t> m_boneIdToChannelId;
		Skeleton *m_skeleton;
	};
};
std::ostream &operator<<(std::ostream &out, const pragma::animation::Pose &o);

#endif
