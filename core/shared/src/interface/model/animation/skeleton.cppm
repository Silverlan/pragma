// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <memory>
#include <unordered_map>

#include <string>

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
	};
};
