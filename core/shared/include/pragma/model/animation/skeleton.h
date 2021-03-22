/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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

using BoneId = uint32_t;
struct DLLNETWORK Bone
	: public std::enable_shared_from_this<Bone>
{
	Bone();
	Bone(const Bone &other); // Parent has to be updated by caller!
	std::string name;
	std::unordered_map<uint32_t,std::shared_ptr<Bone>> children;
	std::weak_ptr<Bone> parent;
	BoneId ID;

	bool IsAncestorOf(const Bone &other) const;
	bool IsDescendantOf(const Bone &other) const;

	bool operator==(const Bone &other) const;
	bool operator!=(const Bone &other) const {return !operator==(other);}
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

namespace udm {struct AssetData;};
class Frame;
class DLLNETWORK Skeleton
{
public:
	static constexpr uint32_t FORMAT_VERSION = 1u;
	static constexpr auto PSKEL_IDENTIFIER = "PSKEL";
	static std::shared_ptr<Skeleton> Load(Frame &reference,const udm::AssetData &data,std::string &outErr);
	Skeleton()=default;
	Skeleton(const Skeleton &other);
	uint32_t AddBone(Bone *bone);
	uint32_t GetBoneCount() const;
	bool IsRootBone(uint32_t boneId) const;
	int32_t LookupBone(const std::string &name) const;
	std::weak_ptr<Bone> GetBone(uint32_t id) const;
	const std::unordered_map<uint32_t,std::shared_ptr<Bone>> &GetRootBones() const;
	std::unordered_map<uint32_t,std::shared_ptr<Bone>> &GetRootBones();
	const std::vector<std::shared_ptr<Bone>> &GetBones() const;
	std::vector<std::shared_ptr<Bone>> &GetBones();

	void Merge(Skeleton &other);
	bool Save(Frame &reference,udm::AssetData &outData,std::string &outErr);

	bool operator==(const Skeleton &other) const;
	bool operator!=(const Skeleton &other) const {return !operator==(other);}
private:
	bool LoadFromAssetData(Frame &reference,const udm::AssetData &data,std::string &outErr);
	std::vector<std::shared_ptr<Bone>> m_bones;
	std::unordered_map<uint32_t,std::shared_ptr<Bone>> m_rootBones;
};
#endif