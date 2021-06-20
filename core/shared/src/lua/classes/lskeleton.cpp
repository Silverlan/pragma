/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lskeleton.h"
#include "luasystem.h"

void Lua::Skeleton::GetRootBones(lua_State *l,::Skeleton &skeleton)
{
	auto &rootBones = skeleton.GetRootBones();

	lua_newtable(l);
	int top = lua_gettop(l);
	for(auto it=rootBones.begin();it!=rootBones.end();it++)
	{
		luabind::object(l,it->second).push(l);
		lua_rawseti(l,top,it->first);
	}
}

void Lua::Skeleton::GetBones(lua_State *l,::Skeleton &skeleton)
{
	auto &bones = skeleton.GetBones();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(int i=0;i<bones.size();i++)
	{
		luabind::object(l,bones[i]).push(l);
		lua_rawseti(l,top,i +1);
	}
}

void Lua::Skeleton::GetBone(lua_State *l,::Skeleton &skeleton,uint32_t boneId)
{
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return;
	Lua::Push(l,bone);
}

void Lua::Skeleton::LookupBone(lua_State *l,::Skeleton &skeleton,const std::string &name) {Lua::PushInt(l,skeleton.LookupBone(name));}

void Lua::Skeleton::AddBone(lua_State *l,::Skeleton &skeleton,const std::string &name,::Bone &parent)
{
	auto *bone = new ::Bone();
	bone->name = name;
	bone->parent = parent.shared_from_this();
	skeleton.AddBone(bone);
	auto ptrBone = skeleton.GetBone(bone->ID).lock();
	Lua::Push(l,ptrBone);
	parent.children[bone->ID] = ptrBone;
}
void Lua::Skeleton::AddBone(lua_State *l,::Skeleton &skeleton,const std::string &name)
{
	auto *bone = new ::Bone();
	bone->name = name;
	skeleton.AddBone(bone);
	auto ptrBone = skeleton.GetBone(bone->ID).lock();
	Lua::Push(l,ptrBone);
}
void Lua::Skeleton::MakeRootBone(lua_State *l,::Skeleton &skeleton,::Bone &bone)
{
	auto &bones = skeleton.GetBones();
	auto it = std::find_if(bones.begin(),bones.end(),[&bone](const std::shared_ptr<::Bone> &boneOther) {
		return &bone == boneOther.get();
	});
	if(it == bones.end())
	{
		Lua::PushBool(l,false);
		return;
	}
	skeleton.GetRootBones()[bone.ID] = bone.shared_from_this();
	Lua::PushBool(l,true);
}
luabind::object Lua::Skeleton::GetBoneHierarchy(lua_State *l,::Skeleton &skeleton)
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
void Lua::Skeleton::GetBoneCount(lua_State *l,::Skeleton &skeleton) {Lua::PushInt(l,skeleton.GetBoneCount());}
void Lua::Skeleton::Merge(lua_State *l,::Skeleton &skeleton,::Skeleton &skeletonOther) {skeleton.Merge(skeletonOther);}
void Lua::Skeleton::ClearBones(lua_State *l,::Skeleton &skeleton) {skeleton.GetBones().clear(); skeleton.GetRootBones().clear();}

/////////////////////////////

void Lua::Bone::GetName(lua_State *l,::Bone &bone)
{
	lua_pushstring(l,bone.name.c_str());
}

void Lua::Bone::GetID(lua_State *l,::Bone &bone)
{
	Lua::PushInt(l,bone.ID);
}

void Lua::Bone::GetChildren(lua_State *l,::Bone &bone)
{
	lua_newtable(l);
	int top = lua_gettop(l);
	for(auto it=bone.children.begin();it!=bone.children.end();it++)
	{
		luabind::object(l,it->second).push(l);
		lua_rawseti(l,top,it->first);
	}
}

void Lua::Bone::GetParent(lua_State *l,::Bone &bone)
{
	auto parent = bone.parent.lock();
	if(parent == nullptr)
		return;
	luabind::object(l,parent).push(l);
}
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
