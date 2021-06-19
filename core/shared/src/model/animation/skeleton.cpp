/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/skeleton.h"
#include "pragma/model/animation/frame.h"
#include <udm.hpp>

Bone::Bone()
	: parent(),ID(0)
{}

Bone::Bone(const Bone &other)
	: ID(other.ID),name{other.name},parent{}
{
	for(auto &pair : other.children)
		children[pair.first] = std::make_shared<Bone>(*pair.second);
	static_assert(sizeof(Bone) == 136,"Update this function when making changes to this class!");
}

bool Bone::IsAncestorOf(const Bone &other) const
{
	if(other.parent.expired())
		return false;
	auto *parent = other.parent.lock().get();
	return (parent == this) ? true : IsAncestorOf(*parent);
}
bool Bone::IsDescendantOf(const Bone &other) const {return other.IsAncestorOf(*this);}
bool Bone::operator==(const Bone &other) const
{
	static_assert(sizeof(Bone) == 136,"Update this function when making changes to this class!");
	if(!(name == other.name && ID == other.ID && children.size() == other.children.size() && parent.expired() == other.parent.expired()))
		return false;
	for(auto &pair : children)
	{
		if(other.children.find(pair.first) == other.children.end())
			return false;
	}
	return true;
}

std::ostream &operator<<(std::ostream &out,const Bone &o)
{
	out<<"Bone";
	out<<"[Name:"<<o.name<<"]";
	out<<"[Id:"<<o.name<<"]";
	out<<"[Children:"<<o.children.size()<<"]";
	out<<"[Parent:";
	if(o.parent.expired())
		out<<"NULL";
	else
		out<<o.parent.lock()->name;
	out<<"]";
	return out;
}

////////////////////////////////////////

uint32_t BoneList::AddBone(const std::string &name)
{
	if(GetBoneID(name) != -1)
		return std::numeric_limits<uint32_t>::max();
	auto ID = static_cast<uint32_t>(m_bones.size());
	m_bones.push_back(name);
	m_boneIDs[name] = ID;
	return ID;
}

uint32_t BoneList::GetBoneCount() const {return static_cast<uint32_t>(m_bones.size());}

int32_t BoneList::GetBoneID(const std::string &name) const
{
	auto it = m_boneIDs.find(name);
	if(it == m_boneIDs.end())
		return -1;
	return it->second;
}

////////////////////////////////////////

std::shared_ptr<Skeleton> Skeleton::Load(Frame &reference,const udm::AssetData &data,std::string &outErr)
{
	auto skeleton = std::make_shared<Skeleton>();
	if(skeleton->LoadFromAssetData(reference,data,outErr) == false)
		return nullptr;
	return skeleton;
}
Skeleton::Skeleton(const Skeleton &other)
{
	m_bones.reserve(other.m_bones.size());
	for(auto &bone : other.m_bones)
		m_bones.push_back(std::make_shared<Bone>(*bone));
	m_rootBones = other.m_rootBones;
	for(auto &pair : m_rootBones)
		pair.second = m_bones[pair.first];
	
	std::function<void(std::unordered_map<uint32_t,std::shared_ptr<Bone>>&,std::shared_ptr<Bone>)> fUpdateHierarchy;
	fUpdateHierarchy = [this,&fUpdateHierarchy](std::unordered_map<uint32_t,std::shared_ptr<Bone>> &bones,std::shared_ptr<Bone> parent) {
		for(auto &pair : bones)
		{
			for(auto &pair : pair.second->children)
				pair.second = m_bones[pair.first];
			fUpdateHierarchy(pair.second->children,pair.second);
			pair.second->parent = parent;
		}
	};
	fUpdateHierarchy(m_rootBones,nullptr);
	static_assert(sizeof(Skeleton) == 88,"Update this function when making changes to this class!");
}

bool Skeleton::IsRootBone(uint32_t boneId) const
{
	return m_rootBones.find(boneId) != m_rootBones.end();
}

int32_t Skeleton::LookupBone(const std::string &name) const
{
	auto &bones = GetBones();
	auto it = std::find_if(bones.begin(),bones.end(),[&name](const std::shared_ptr<Bone> &bone) {
		return (bone->name == name) ? true : false;
	});
	if(it != bones.end())
		return static_cast<int32_t>(it -bones.begin());
	return -1;
}

uint32_t Skeleton::AddBone(Bone *bone)
{
	auto ID = static_cast<uint32_t>(m_bones.size());
	bone->ID = ID;
	m_bones.push_back(std::shared_ptr<Bone>(bone));
	return ID;
}

const std::vector<std::shared_ptr<Bone>> &Skeleton::GetBones() const {return const_cast<Skeleton*>(this)->GetBones();}
std::vector<std::shared_ptr<Bone>> &Skeleton::GetBones() {return m_bones;}

uint32_t Skeleton::GetBoneCount() const {return static_cast<uint32_t>(m_bones.size());}

std::unordered_map<uint32_t,std::shared_ptr<Bone>> &Skeleton::GetRootBones() {return m_rootBones;}
const std::unordered_map<uint32_t,std::shared_ptr<Bone>> &Skeleton::GetRootBones() const {return m_rootBones;}

std::weak_ptr<Bone> Skeleton::GetBone(uint32_t id) const
{
	if(id >= m_bones.size())
		return {};
	return m_bones[id];
}

bool Skeleton::operator==(const Skeleton &other) const
{
	static_assert(sizeof(Skeleton) == 88,"Update this function when making changes to this class!");
	if(!(m_bones.size() == other.m_bones.size() && m_rootBones.size() == other.m_rootBones.size()))
		return false;
	for(auto i=decltype(m_bones.size()){0u};i<m_bones.size();++i)
	{
		if(*m_bones[i] != *other.m_bones[i])
			return false;
	}
	for(auto &pair : m_rootBones)
	{
		if(other.m_rootBones.find(pair.first) == other.m_rootBones.end())
			return false;
	}
	return true;
}

bool Skeleton::LoadFromAssetData(Frame &reference,const udm::AssetData &data,std::string &outErr)
{
	if(data.GetAssetType() != PSKEL_IDENTIFIER)
	{
		outErr = "Incorrect format!";
		return false;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1)
	{
		outErr = "Invalid version!";
		return false;
	}

	struct BoneInfo
	{
		udm::LinkedPropertyWrapper udmBone;
		std::string_view name;
		std::vector<BoneId> childIds;
		BoneId index;
	};

	std::vector<BoneInfo> udmBoneList {};
	std::function<BoneId(udm::LinkedPropertyWrapper &prop,const std::string_view &name)> readBone = nullptr;
	readBone = [this,&reference,&readBone,&udmBoneList](udm::LinkedPropertyWrapper &udmBone,const std::string_view &name) -> BoneId {
		if(udmBoneList.size() == udmBoneList.capacity())
			udmBoneList.reserve(udmBoneList.size() *1.5 +50);

		auto i = udmBoneList.size();
		udmBoneList.push_back({});
		auto &boneInfo = udmBoneList.back();
		boneInfo.udmBone = const_cast<const udm::LinkedPropertyWrapper&>(udmBone);
		boneInfo.name = name;
		uint32_t idx = 0;
		udmBone["index"](idx);
		boneInfo.index = idx;

		auto udmChildren = udmBone["children"];
		boneInfo.childIds.reserve(udmChildren.GetChildCount());
		for(auto udmChild : udmChildren.ElIt())
		{
			auto childBoneIdx = readBone(udmChild.property,udmChild.key);
			udmBoneList[i].childIds.push_back(childBoneIdx);
		}
		return idx;
	};
	auto udmBones = udm["bones"];
	std::vector<BoneId> rootBoneIndices {};
	rootBoneIndices.reserve(udmBones.GetChildCount());
	for(auto udmBone : udmBones.ElIt())
		rootBoneIndices.push_back(readBone(udmBone.property,udmBone.key));

	auto &bones = GetBones();
	auto numBones = udmBoneList.size();
	bones.resize(numBones);
	reference.SetBoneCount(numBones);
	for(auto i=decltype(udmBoneList.size()){0u};i<udmBoneList.size();++i)
	{
		auto &boneInfo = udmBoneList[i];
		bones[boneInfo.index] = std::make_shared<Bone>();
		bones[boneInfo.index]->ID = boneInfo.index;
	}

	for(auto i=decltype(udmBoneList.size()){0u};i<udmBoneList.size();++i)
	{
		auto &boneInfo = udmBoneList[i];
		auto &bone = bones[boneInfo.index];
		bone->name = boneInfo.name;

		umath::ScaledTransform pose {};
		boneInfo.udmBone["pose"](pose);
		auto &scale = pose.GetScale();
		if(reference.GetBoneScales().empty() && scale != Vector3{1.f,1.f,1.f})
			reference.GetBoneScales().resize(numBones,Vector3{1.f,1.f,1.f});
		reference.SetBonePose(boneInfo.index,pose);

		bone->children.reserve(boneInfo.childIds.size());
		for(auto id : boneInfo.childIds)
		{
			bone->children.insert(std::make_pair(id,bones[id]));
			bones[id]->parent = bone;
		}
	}

	auto &rootBones = GetRootBones();
	for(auto idx : rootBoneIndices)
		rootBones[idx] = bones[idx];
	return true;
}

bool Skeleton::Save(Frame &reference,udm::AssetDataArg outData,std::string &outErr)
{
	outData.SetAssetType(PSKEL_IDENTIFIER);
	outData.SetAssetVersion(FORMAT_VERSION);
	auto udm = *outData;

	std::function<void(udm::LinkedPropertyWrapperArg prop,const Bone &bone)> writeBone = nullptr;
	writeBone = [this,&reference,&writeBone](udm::LinkedPropertyWrapperArg prop,const Bone &bone) {
		auto udmBone = prop[bone.name];
		udmBone["index"] = static_cast<uint32_t>(bone.ID);
		umath::ScaledTransform transform;
		reference.GetBonePose(bone.ID,transform);
		udmBone["pose"] = transform;

		for(auto &pair : bone.children)
			writeBone(udmBone["children"],*pair.second);
	};
	auto udmBones = udm["bones"];
	for(auto &pair : m_rootBones)
		writeBone(udmBones,*pair.second);
	return true;
}

void Skeleton::Merge(Skeleton &other)
{
	auto &bones = GetBones();
	auto &rootBones = GetRootBones();
	std::function<void(const std::unordered_map<uint32_t,std::shared_ptr<Bone>>&,std::shared_ptr<Bone>)> mergeHierarchy = nullptr;
	mergeHierarchy = [this,&bones,&rootBones,&mergeHierarchy](const std::unordered_map<uint32_t,std::shared_ptr<Bone>> &otherBones,std::shared_ptr<Bone> parent) {
		for(auto &pair : otherBones)
		{
			auto &otherBone = pair.second;
			auto it = std::find_if(bones.begin(),bones.end(),[&otherBone](const std::shared_ptr<Bone> &bone) {
				return ustring::compare(bone->name,otherBone->name,true);
			});
			if(it == bones.end())
			{
				// Bone doesn't exist yet; Add to hierarchy
				bones.push_back(std::make_shared<Bone>());
				auto &newBone = bones.back();
				newBone->ID = bones.size() -1;
				newBone->name = otherBone->name;
				if(parent != nullptr)
				{
					newBone->parent = parent;
					parent->children.insert(std::make_pair(newBone->ID,newBone));
				}
				else
					rootBones.insert(std::make_pair(newBone->ID,newBone));
				mergeHierarchy(otherBone->children,newBone);
			}
			else // Bone already exists; Ignore it and continue in hierarchy
				mergeHierarchy(otherBone->children,parent);
		}
	};
	auto &otherRootBones = other.GetRootBones();
	mergeHierarchy(otherRootBones,nullptr);
}
