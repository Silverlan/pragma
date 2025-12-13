// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.animation.pose;

export import :model.animation.bone;
export import :model.animation.enums;
import panima;

export {
	namespace pragma::animation {
		class Skeleton;
		class DLLNETWORK Pose {
		  public:
			Pose() = default;
			Pose(const Pose &) = default;
			Pose(Pose &&) = default;
			Pose(const std::vector<math::ScaledTransform> &transforms);
			Pose(std::vector<math::ScaledTransform> &&transforms);
			Pose &operator=(const Pose &) = default;
			Pose &operator=(Pose &&) = default;
			bool operator==(const Pose &other) const { return m_transforms == other.m_transforms; }
			bool operator!=(const Pose &other) const { return !operator==(other); }
			const std::vector<math::ScaledTransform> &GetTransforms() const { return const_cast<Pose *>(this)->GetTransforms(); }
			std::vector<math::ScaledTransform> &GetTransforms() { return m_transforms; }
			void SetTransformCount(uint32_t c);
			void SetBoneIndex(uint32_t channelId, BoneId boneId);
			math::ScaledTransform *GetTransform(BoneId idx);
			const math::ScaledTransform *GetTransform(BoneId idx) const { return const_cast<Pose *>(this)->GetTransform(idx); }
			void SetTransform(BoneId idx, const math::ScaledTransform &pose);
			void Clear();
			void Lerp(const Pose &other, float f);
			operator bool() const { return !m_transforms.empty(); }

			void Localize(const Skeleton &skeleton);
			void Globalize(const Skeleton &skeleton);

			std::vector<uint32_t> &GetBoneTranslationTable() { return m_boneIdToChannelId; }
			const std::vector<uint32_t> &GetBoneTranslationTable() const { return const_cast<Pose *>(this)->GetBoneTranslationTable(); }
		  private:
			uint32_t GetChannelIdx(BoneId boneId) const;
			std::vector<math::ScaledTransform> m_transforms {};
			std::vector<uint32_t> m_boneIdToChannelId;
			Skeleton *m_skeleton;
		};
	};
	std::ostream &operator<<(std::ostream &out, const pragma::animation::Pose &o);
};
