/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PRAGMA_SKELETON_HPP__
#define __PRAGMA_SKELETON_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/model/animation/bone.hpp"
#include <cinttypes>
#include <memory>
#include <unordered_map>
#include <mathutil/transform.hpp>

namespace udm {
	struct AssetData;
	using AssetDataArg = const AssetData &;
};
namespace pragma::animation {
	struct Bone;
	class DLLNETWORK Skeleton {
	  public:
		static constexpr uint32_t FORMAT_VERSION = 1u;
		static constexpr auto PSKEL_IDENTIFIER = "PSKEL";
		static std::shared_ptr<Skeleton> Load(const udm::AssetData &data, std::string &outErr);
		Skeleton() = default;
		Skeleton(const Skeleton &other);
		BoneId AddBone(Bone *bone);
		uint32_t GetBoneCount() const;
		bool IsRootBone(pragma::animation::BoneId boneId) const;
		int32_t LookupBone(const std::string &name) const;
		std::weak_ptr<Bone> GetBone(pragma::animation::BoneId id) const;
		const std::unordered_map<pragma::animation::BoneId, std::shared_ptr<Bone>> &GetRootBones() const;
		std::unordered_map<pragma::animation::BoneId, std::shared_ptr<Bone>> &GetRootBones();
		const std::vector<std::shared_ptr<Bone>> &GetBones() const;
		std::vector<std::shared_ptr<Bone>> &GetBones();
		std::vector<umath::ScaledTransform> &GetBonePoses() { return m_referencePoses; }
		const std::vector<umath::ScaledTransform> &GetBonePoses() const { return const_cast<Skeleton *>(this)->GetBonePoses(); }

		bool TransformToParentSpace(const std::vector<umath::ScaledTransform> &gsPoses, std::vector<umath::ScaledTransform> &outPoses) const;
		bool TransformToGlobalSpace(const std::vector<umath::ScaledTransform> &psPoses, std::vector<umath::ScaledTransform> &outPoses) const;

		void Merge(Skeleton &other);
		bool Save(udm::AssetDataArg outData, std::string &outErr);

		bool operator==(const Skeleton &other) const;
		bool operator!=(const Skeleton &other) const { return !operator==(other); }
	  private:
		bool LoadFromAssetData(const udm::AssetData &data, std::string &outErr);
		std::vector<std::shared_ptr<Bone>> m_bones;
		std::unordered_map<pragma::animation::BoneId, std::shared_ptr<Bone>> m_rootBones;
		std::vector<umath::ScaledTransform> m_referencePoses;
		std::unordered_map<pragma::animation::BoneId, BoneMetaData> m_boneMetadata;
	};
};

#endif
