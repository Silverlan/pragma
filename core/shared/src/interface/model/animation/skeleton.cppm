// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.animation.skeleton;

export import :model.animation.bone;
export import :model.animation.enums;
export import pragma.udm;

export namespace pragma::animation {
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
		bool IsRootBone(BoneId boneId) const;
		int32_t LookupBone(const std::string &name) const;
		std::weak_ptr<Bone> GetBone(BoneId id) const;
		const std::unordered_map<BoneId, std::shared_ptr<Bone>> &GetRootBones() const;
		std::unordered_map<BoneId, std::shared_ptr<Bone>> &GetRootBones();
		const std::vector<std::shared_ptr<Bone>> &GetBones() const;
		std::vector<std::shared_ptr<Bone>> &GetBones();
		std::vector<math::ScaledTransform> &GetBonePoses() { return m_referencePoses; }
		const std::vector<math::ScaledTransform> &GetBonePoses() const { return const_cast<Skeleton *>(this)->GetBonePoses(); }

		bool TransformToParentSpace(const std::vector<math::ScaledTransform> &gsPoses, std::vector<math::ScaledTransform> &outPoses) const;
		bool TransformToGlobalSpace(const std::vector<math::ScaledTransform> &psPoses, std::vector<math::ScaledTransform> &outPoses) const;

		void Merge(Skeleton &other);
		bool Save(udm::AssetDataArg outData, std::string &outErr);

		bool operator==(const Skeleton &other) const;
		bool operator!=(const Skeleton &other) const { return !operator==(other); }
	  private:
		bool LoadFromAssetData(const udm::AssetData &data, std::string &outErr);
		std::vector<std::shared_ptr<Bone>> m_bones;
		std::unordered_map<BoneId, std::shared_ptr<Bone>> m_rootBones;
		std::vector<math::ScaledTransform> m_referencePoses;
	};
};
