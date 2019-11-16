#include "stdafx_shared.h"
#include "pragma/model/animation/skeleton.h"

Bone::Bone()
	: parent(),ID(0)
{}

Bone::Bone(const Bone &other)
	: ID(other.ID),parent{}
{
	for(auto &pair : other.children)
		children[pair.first] = std::make_shared<Bone>(*pair.second);
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

Skeleton::Skeleton(const Skeleton &other)
{
	m_bones.reserve(other.m_bones.size());
	for(auto &bone : other.m_bones)
		m_bones.push_back(std::make_shared<Bone>(*bone));
	m_rootBones = other.m_rootBones;
	for(auto &pair : m_rootBones)
		pair.second = m_bones[pair.first];
	
	std::function<void(std::unordered_map<uint32_t,std::shared_ptr<Bone>>&,std::shared_ptr<Bone>)> fUpdateHierarchy;
	fUpdateHierarchy = [&fUpdateHierarchy](std::unordered_map<uint32_t,std::shared_ptr<Bone>> &bones,std::shared_ptr<Bone> parent) {
		for(auto &pair : bones)
		{
			fUpdateHierarchy(pair.second->children,pair.second);
			pair.second->parent = parent;
		}
	};
	fUpdateHierarchy(m_rootBones,nullptr);
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
