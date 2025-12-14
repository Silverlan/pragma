// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.animation.skeleton;

std::shared_ptr<pragma::animation::Skeleton> pragma::animation::Skeleton::Load(const udm::AssetData &data, std::string &outErr)
{
	auto skeleton = pragma::util::make_shared<Skeleton>();
	if(skeleton->LoadFromAssetData(data, outErr) == false)
		return nullptr;
	return skeleton;
}
pragma::animation::Skeleton::Skeleton(const Skeleton &other)
{
	m_bones.reserve(other.m_bones.size());
	for(auto &bone : other.m_bones)
		m_bones.push_back(pragma::util::make_shared<Bone>(*bone));
	m_rootBones = other.m_rootBones;
	for(auto &pair : m_rootBones)
		pair.second = m_bones[pair.first];
	m_referencePoses = other.m_referencePoses;

	std::function<void(std::unordered_map<BoneId, std::shared_ptr<Bone>> &, std::shared_ptr<Bone>)> fUpdateHierarchy;
	fUpdateHierarchy = [this, &fUpdateHierarchy](std::unordered_map<BoneId, std::shared_ptr<Bone>> &bones, std::shared_ptr<Bone> parent) {
		for(auto &pair : bones) {
			for(auto &pair : pair.second->children)
				pair.second = m_bones[pair.first];
			fUpdateHierarchy(pair.second->children, pair.second);
			pair.second->parent = parent;
		}
	};
	fUpdateHierarchy(m_rootBones, nullptr);
#ifdef _MSC_VER
	static_assert(sizeof(Skeleton) == 112, "Update this function when making changes to this class!");
#endif
}

bool pragma::animation::Skeleton::IsRootBone(BoneId boneId) const { return m_rootBones.find(boneId) != m_rootBones.end(); }

bool pragma::animation::Skeleton::TransformToParentSpace(const std::vector<math::ScaledTransform> &gsPoses, std::vector<math::ScaledTransform> &outPoses) const
{
	if(gsPoses.size() != outPoses.size() || gsPoses.size() != m_bones.size())
		return false;
	if(&outPoses == &gsPoses) {
		auto tmp = gsPoses;
		return TransformToParentSpace(tmp, outPoses);
	}
	std::function<void(const Bone &bone)> transformToParentSpace = nullptr;
	transformToParentSpace = [&](const Bone &bone) {
		if(bone.parent.expired() == false)
			outPoses[bone.ID] = gsPoses[bone.parent.lock()->ID].GetInverse() * gsPoses[bone.ID];
		for(auto &pair : bone.children)
			transformToParentSpace(*pair.second);
	};
	for(auto &pair : m_rootBones)
		transformToParentSpace(*pair.second);
	return true;
}
bool pragma::animation::Skeleton::TransformToGlobalSpace(const std::vector<math::ScaledTransform> &psPoses, std::vector<math::ScaledTransform> &outPoses) const
{
	if(psPoses.size() != outPoses.size() || psPoses.size() != m_bones.size())
		return false;
	if(&outPoses == &psPoses) {
		auto tmp = psPoses;
		return TransformToParentSpace(tmp, outPoses);
	}
	std::function<void(const Bone &bone)> transformToParentSpace = nullptr;
	transformToParentSpace = [&](const Bone &bone) {
		if(bone.parent.expired() == false)
			outPoses[bone.ID] = psPoses[bone.parent.lock()->ID] * psPoses[bone.ID];
		for(auto &pair : bone.children)
			transformToParentSpace(*pair.second);
	};
	for(auto &pair : m_rootBones)
		transformToParentSpace(*pair.second);
	return true;
}

int32_t pragma::animation::Skeleton::LookupBone(const std::string &name) const
{
	auto &bones = GetBones();
	auto it = std::find_if(bones.begin(), bones.end(), [&name](const std::shared_ptr<Bone> &bone) { return (bone->name == name) ? true : false; });
	if(it != bones.end())
		return static_cast<int32_t>(it - bones.begin());
	return -1;
}

pragma::animation::BoneId pragma::animation::Skeleton::AddBone(Bone *bone)
{
	auto ID = static_cast<uint32_t>(m_bones.size());
	bone->ID = ID;
	m_bones.push_back(std::shared_ptr<Bone>(bone));

	if(bone->name.empty())
		bone->name = "unnamed_" + std::to_string(ID);
	return ID;
}

const std::vector<std::shared_ptr<pragma::animation::Bone>> &pragma::animation::Skeleton::GetBones() const { return const_cast<Skeleton *>(this)->GetBones(); }
std::vector<std::shared_ptr<pragma::animation::Bone>> &pragma::animation::Skeleton::GetBones() { return m_bones; }

uint32_t pragma::animation::Skeleton::GetBoneCount() const { return static_cast<uint32_t>(m_bones.size()); }

std::unordered_map<pragma::animation::BoneId, std::shared_ptr<pragma::animation::Bone>> &pragma::animation::Skeleton::GetRootBones() { return m_rootBones; }
const std::unordered_map<pragma::animation::BoneId, std::shared_ptr<pragma::animation::Bone>> &pragma::animation::Skeleton::GetRootBones() const { return m_rootBones; }

std::weak_ptr<pragma::animation::Bone> pragma::animation::Skeleton::GetBone(BoneId id) const
{
	if(id >= m_bones.size())
		return {};
	return m_bones[id];
}

bool pragma::animation::Skeleton::operator==(const Skeleton &other) const
{
#ifdef _MSC_VER
	static_assert(sizeof(Skeleton) == 112, "Update this function when making changes to this class!");
#endif
	if(!(m_bones.size() == other.m_bones.size() && m_rootBones.size() == other.m_rootBones.size()))
		return false;
	for(auto i = decltype(m_bones.size()) {0u}; i < m_bones.size(); ++i) {
		if(*m_bones[i] != *other.m_bones[i])
			return false;
	}
	for(auto &pair : m_rootBones) {
		if(other.m_rootBones.find(pair.first) == other.m_rootBones.end())
			return false;
	}
	return true;
}

bool pragma::animation::Skeleton::LoadFromAssetData(const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != PSKEL_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}

	struct BoneInfo {
		udm::LinkedPropertyWrapper udmBone;
		std::string_view name;
		std::vector<BoneId> childIds;
		BoneId index;
	};

	std::vector<BoneInfo> udmBoneList {};
	std::function<BoneId(udm::LinkedPropertyWrapper & prop, const std::string_view &name)> readBone = nullptr;
	readBone = [this, &readBone, &udmBoneList](udm::LinkedPropertyWrapper &udmBone, const std::string_view &name) -> BoneId {
		if(udmBoneList.size() == udmBoneList.capacity())
			udmBoneList.reserve(udmBoneList.size() * 1.5 + 50);

		auto i = udmBoneList.size();
		udmBoneList.push_back({});
		auto &boneInfo = udmBoneList.back();
		boneInfo.udmBone = const_cast<const udm::LinkedPropertyWrapper &>(udmBone);
		boneInfo.name = name;
		uint32_t idx = 0;
		udmBone["index"](idx);
		boneInfo.index = idx;

		auto udmChildren = udmBone["children"];
		boneInfo.childIds.reserve(udmChildren.GetChildCount());
		for(auto udmChild : udmChildren.ElIt()) {
			auto childBoneIdx = readBone(udmChild.property, udmChild.key);
			udmBoneList[i].childIds.push_back(childBoneIdx);
		}
		return idx;
	};
	auto udmBones = udm["bones"];
	std::vector<BoneId> rootBoneIndices {};
	rootBoneIndices.reserve(udmBones.GetChildCount());
	for(auto udmBone : udmBones.ElIt())
		rootBoneIndices.push_back(readBone(udmBone.property, udmBone.key));

	auto &bones = GetBones();
	auto numBones = udmBoneList.size();
	bones.resize(numBones);
	m_referencePoses.resize(numBones);
	for(auto i = decltype(udmBoneList.size()) {0u}; i < udmBoneList.size(); ++i) {
		auto &boneInfo = udmBoneList[i];
		if(boneInfo.index >= bones.size()) {
			outErr = "Bone index is out of bounds of bone list!";
			return false;
		}
		bones[boneInfo.index] = pragma::util::make_shared<Bone>();
		bones[boneInfo.index]->ID = boneInfo.index;
	}

	for(auto i = decltype(udmBoneList.size()) {0u}; i < udmBoneList.size(); ++i) {
		auto &boneInfo = udmBoneList[i];
		auto &bone = bones[boneInfo.index];
		bone->name = boneInfo.name;

		if(boneInfo.index >= m_referencePoses.size())
			throw std::runtime_error {"Bone index " + std::to_string(boneInfo.index) + " for reference pose is out of bounds!"};
		auto &pose = m_referencePoses[boneInfo.index];
		boneInfo.udmBone["pose"](pose);

		bone->children.reserve(boneInfo.childIds.size());
		for(auto id : boneInfo.childIds) {
			bone->children.insert(std::make_pair(id, bones[id]));
			bones[id]->parent = bone;
		}
	}

	auto &rootBones = GetRootBones();
	for(auto idx : rootBoneIndices)
		rootBones[idx] = bones[idx];
	return true;
}

bool pragma::animation::Skeleton::Save(udm::AssetDataArg outData, std::string &outErr)
{
	outData.SetAssetType(PSKEL_IDENTIFIER);
	outData.SetAssetVersion(FORMAT_VERSION);
	auto udm = *outData;

	std::function<bool(udm::LinkedPropertyWrapperArg prop, const Bone &bone)> writeBone = nullptr;
	writeBone = [this, &writeBone, &outErr](udm::LinkedPropertyWrapperArg prop, const Bone &bone) -> bool {
		std::string name = bone.name;
		if(prop[name]) {
			outErr = "Duplicate bone '" + name + "'.";
			return false;
		}
		auto udmBone = prop[name];
		udmBone["index"] = static_cast<uint32_t>(bone.ID);
		udmBone["pose"] = m_referencePoses[bone.ID];

		for(auto &pair : bone.children) {
			if(!writeBone(udmBone["children"], *pair.second))
				return false;
		}
		return true;
	};
	auto udmBones = udm["bones"];
	for(auto &pair : m_rootBones) {
		if(!writeBone(udmBones, *pair.second))
			return false;
	}
	return true;
}

void pragma::animation::Skeleton::Merge(Skeleton &other)
{
	auto &bones = GetBones();
	auto &rootBones = GetRootBones();
	std::function<void(const std::unordered_map<BoneId, std::shared_ptr<Bone>> &, std::shared_ptr<Bone>)> mergeHierarchy = nullptr;
	mergeHierarchy = [this, &bones, &rootBones, &mergeHierarchy](const std::unordered_map<BoneId, std::shared_ptr<Bone>> &otherBones, std::shared_ptr<Bone> parent) {
		for(auto &pair : otherBones) {
			auto &otherBone = pair.second;
			auto it = std::find_if(bones.begin(), bones.end(), [&otherBone](const std::shared_ptr<Bone> &bone) { return string::compare(bone->name.c_str(), otherBone->name.c_str(), true); });
			if(it == bones.end()) {
				// Bone doesn't exist yet; Add to hierarchy
				bones.push_back(pragma::util::make_shared<Bone>());
				auto &newBone = bones.back();
				newBone->ID = bones.size() - 1;
				newBone->name = otherBone->name;
				if(parent != nullptr) {
					newBone->parent = parent;
					parent->children.insert(std::make_pair(newBone->ID, newBone));
				}
				else
					rootBones.insert(std::make_pair(newBone->ID, newBone));
				mergeHierarchy(otherBone->children, newBone);
			}
			else // Bone already exists; Ignore it and continue in hierarchy
				mergeHierarchy(otherBone->children, parent);
		}
	};
	auto &otherRootBones = other.GetRootBones();
	mergeHierarchy(otherRootBones, nullptr);
}
