#ifndef __SKELETON_H__
#define __SKELETON_H__
#include "pragma/networkdefinitions.h"
#include <iostream>
#include <vector>
#include <mathutil/glmutil.h>
#include <string>
#include <unordered_map>
#include "pragma/math/orientation.h"
#include "pragma/physics/jointinfo.h"

struct DLLNETWORK Bone
	: public std::enable_shared_from_this<Bone>
{
	Bone();
	Bone(const Bone &other); // Parent has to be updated by caller!
	std::string name;
	std::unordered_map<uint32_t,std::shared_ptr<Bone>> children;
	std::weak_ptr<Bone> parent;
	uint32_t ID;
};

struct DLLNETWORK BoneList // Simplified Skeleton without an hierarchy
{
private:
	std::vector<std::string> m_bones;
	std::unordered_map<std::string,uint32_t> m_boneIDs;
public:
	uint32_t AddBone(const std::string &name);
	int32_t GetBoneID(const std::string &name) const;
	uint32_t GetBoneCount() const;
};

class DLLNETWORK Skeleton
{
private:
	std::vector<std::shared_ptr<Bone>> m_bones;
	std::unordered_map<uint32_t,std::shared_ptr<Bone>> m_rootBones;
public:
	Skeleton()=default;
	Skeleton(const Skeleton &other);
	uint32_t AddBone(Bone *bone);
	uint32_t GetBoneCount() const;
	int32_t LookupBone(const std::string &name) const;
	std::weak_ptr<Bone> GetBone(uint32_t id) const;
	const std::unordered_map<uint32_t,std::shared_ptr<Bone>> &GetRootBones() const;
	std::unordered_map<uint32_t,std::shared_ptr<Bone>> &GetRootBones();
	const std::vector<std::shared_ptr<Bone>> &GetBones() const;
	std::vector<std::shared_ptr<Bone>> &GetBones();

	void Merge(Skeleton &other);
};
#endif