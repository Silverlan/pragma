/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lskeleton.h"
#include "luasystem.h"

bool Lua::Skeleton::IsRootBone(lua_State *l,::Skeleton &skeleton,const std::string &boneName)
{
	auto boneId = skeleton.LookupBone(boneName);
	return IsRootBone(l,skeleton,boneId);
}
bool Lua::Skeleton::IsRootBone(lua_State *l,::Skeleton &skeleton,uint32_t boneId)
{
	auto &rootBones = skeleton.GetRootBones();
	return rootBones.find(boneId) != rootBones.end();
}

luabind::map<uint32_t,std::shared_ptr<::Bone>> Lua::Skeleton::GetRootBones(lua_State *l,::Skeleton &skeleton)
{
	auto &rootBones = skeleton.GetRootBones();
	return Lua::map_to_table(l,rootBones);
}

luabind::tableT<std::shared_ptr<::Bone>> Lua::Skeleton::GetBones(lua_State *l,::Skeleton &skeleton)
{
	auto &bones = skeleton.GetBones();
	return Lua::vector_to_table(l,bones);
}

std::shared_ptr<::Bone> Lua::Skeleton::GetBone(lua_State *l,::Skeleton &skeleton,uint32_t boneId)
{
	return skeleton.GetBone(boneId).lock();
}

int32_t Lua::Skeleton::LookupBone(lua_State *l,::Skeleton &skeleton,const std::string &name) {return skeleton.LookupBone(name);}

std::shared_ptr<::Bone> Lua::Skeleton::AddBone(lua_State *l,::Skeleton &skeleton,const std::string &name,::Bone &parent)
{
	auto *bone = new ::Bone();
	bone->name = name;
	bone->parent = parent.shared_from_this();
	skeleton.AddBone(bone);
	auto ptrBone = skeleton.GetBone(bone->ID).lock();
	parent.children[bone->ID] = ptrBone;
	return ptrBone;
}
std::shared_ptr<::Bone> Lua::Skeleton::AddBone(lua_State *l,::Skeleton &skeleton,const std::string &name)
{
	auto *bone = new ::Bone();
	bone->name = name;
	skeleton.AddBone(bone);
	auto ptrBone = skeleton.GetBone(bone->ID).lock();
	return ptrBone;
}
bool Lua::Skeleton::MakeRootBone(lua_State *l,::Skeleton &skeleton,::Bone &bone)
{
	auto &bones = skeleton.GetBones();
	auto it = std::find_if(bones.begin(),bones.end(),[&bone](const std::shared_ptr<::Bone> &boneOther) {
		return &bone == boneOther.get();
	});
	if(it == bones.end())
		return false;
	skeleton.GetRootBones()[bone.ID] = bone.shared_from_this();
	return true;
}
luabind::map<uint16_t,luabind::tableT<void>> Lua::Skeleton::GetBoneHierarchy(lua_State *l,::Skeleton &skeleton)
{
	auto t = luabind::newtable(l);
	std::function<void(const ::Bone&,const luabind::object&)> fGetHierarchy = nullptr;
	fGetHierarchy = [l,&fGetHierarchy](const ::Bone &bone,const luabind::object &t) {
		t[bone.ID] = luabind::newtable(l);
		for(auto &pair : bone.children)
			fGetHierarchy(*pair.second,t[bone.ID]);
	};
	for(auto &pair : skeleton.GetRootBones())
		fGetHierarchy(*pair.second,t);
	return t;
}
uint32_t Lua::Skeleton::GetBoneCount(lua_State *l,::Skeleton &skeleton) {return skeleton.GetBoneCount();}
void Lua::Skeleton::Merge(lua_State *l,::Skeleton &skeleton,::Skeleton &skeletonOther) {skeleton.Merge(skeletonOther);}
void Lua::Skeleton::ClearBones(lua_State *l,::Skeleton &skeleton) {skeleton.GetBones().clear(); skeleton.GetRootBones().clear();}

/////////////////////////////

std::string Lua::Bone::GetName(lua_State *l,::Bone &bone) {return bone.name;}

BoneId Lua::Bone::GetID(lua_State *l,::Bone &bone) {return bone.ID;}

luabind::map<BoneId,std::shared_ptr<::Bone>> Lua::Bone::GetChildren(lua_State *l,::Bone &bone)
{
	auto t = luabind::newtable(l);
	for(auto &pair : bone.children)
		t[pair.first] = pair.second;
	return t;
}

std::shared_ptr<::Bone> Lua::Bone::GetParent(lua_State *l,::Bone &bone) {return bone.parent.lock();}
void Lua::Bone::SetName(lua_State *l,::Bone &bone,const std::string &name) {bone.name = name;}
void Lua::Bone::SetParent(lua_State *l,::Bone &bone,::Bone &parent)
{
	ClearParent(l,bone);
	bone.parent = parent.shared_from_this();
	parent.children[bone.ID] = bone.shared_from_this();
}
void Lua::Bone::ClearParent(lua_State *l,::Bone &bone)
{
	if(bone.parent.expired() == false)
	{
		auto prevParent = bone.parent.lock();
		auto it = prevParent->children.find(bone.ID);
		if(it != prevParent->children.end())
			prevParent->children.erase(it);
	}
	bone.parent = {};
}
