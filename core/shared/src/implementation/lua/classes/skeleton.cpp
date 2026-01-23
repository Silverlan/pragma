// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.skeleton;

bool Lua::Skeleton::IsRootBone(lua::State *l, pragma::animation::Skeleton &skeleton, const std::string &boneName)
{
	auto boneId = skeleton.LookupBone(boneName);
	return IsRootBone(l, skeleton, boneId);
}
bool Lua::Skeleton::IsRootBone(lua::State *l, pragma::animation::Skeleton &skeleton, uint32_t boneId)
{
	auto &rootBones = skeleton.GetRootBones();
	return rootBones.find(boneId) != rootBones.end();
}

luabind::map<uint32_t, std::shared_ptr<pragma::animation::Bone>> Lua::Skeleton::GetRootBones(lua::State *l, pragma::animation::Skeleton &skeleton)
{
	auto &rootBones = skeleton.GetRootBones();
	return map_to_table(l, rootBones);
}

luabind::tableT<std::shared_ptr<pragma::animation::Bone>> Lua::Skeleton::GetBones(lua::State *l, pragma::animation::Skeleton &skeleton)
{
	auto &bones = skeleton.GetBones();
	return vector_to_table(l, bones);
}

std::shared_ptr<pragma::animation::Bone> Lua::Skeleton::GetBone(lua::State *l, pragma::animation::Skeleton &skeleton, uint32_t boneId) { return skeleton.GetBone(boneId).lock(); }

int32_t Lua::Skeleton::LookupBone(lua::State *l, pragma::animation::Skeleton &skeleton, const std::string &name) { return skeleton.LookupBone(name); }

std::shared_ptr<pragma::animation::Bone> Lua::Skeleton::AddBone(lua::State *l, pragma::animation::Skeleton &skeleton, const std::string &name, pragma::animation::Bone &parent)
{
	auto *bone = new pragma::animation::Bone();
	bone->name = name;
	bone->parent = parent.shared_from_this();
	skeleton.AddBone(bone);
	auto ptrBone = skeleton.GetBone(bone->ID).lock();
	parent.children[bone->ID] = ptrBone;
	return ptrBone;
}
std::shared_ptr<pragma::animation::Bone> Lua::Skeleton::AddBone(lua::State *l, pragma::animation::Skeleton &skeleton, const std::string &name)
{
	auto *bone = new pragma::animation::Bone();
	bone->name = name;
	skeleton.AddBone(bone);
	auto ptrBone = skeleton.GetBone(bone->ID).lock();
	return ptrBone;
}
bool Lua::Skeleton::MakeRootBone(lua::State *l, pragma::animation::Skeleton &skeleton, pragma::animation::Bone &bone)
{
	auto &bones = skeleton.GetBones();
	auto it = std::find_if(bones.begin(), bones.end(), [&bone](const std::shared_ptr<pragma::animation::Bone> &boneOther) { return &bone == boneOther.get(); });
	if(it == bones.end())
		return false;
	skeleton.GetRootBones()[bone.ID] = bone.shared_from_this();
	return true;
}
luabind::map<uint16_t, luabind::tableT<void>> Lua::Skeleton::GetBoneHierarchy(lua::State *l, pragma::animation::Skeleton &skeleton)
{
	auto t = luabind::newtable(l);
	std::function<void(const pragma::animation::Bone &, const luabind::object &)> fGetHierarchy = nullptr;
	fGetHierarchy = [l, &fGetHierarchy](const pragma::animation::Bone &bone, const luabind::object &t) {
		t[bone.ID] = luabind::newtable(l);
		for(auto &pair : bone.children)
			fGetHierarchy(*pair.second, t[bone.ID]);
	};
	for(auto &pair : skeleton.GetRootBones())
		fGetHierarchy(*pair.second, t);
	return t;
}
void Lua::Skeleton::ClearBones(lua::State *l, pragma::animation::Skeleton &skeleton)
{
	skeleton.GetBones().clear();
	skeleton.GetRootBones().clear();
}

/////////////////////////////

std::string Lua::Bone::GetName(lua::State *l, pragma::animation::Bone &bone) { return bone.name; }

pragma::animation::BoneId Lua::Bone::GetID(lua::State *l, pragma::animation::Bone &bone) { return bone.ID; }

luabind::map<pragma::animation::BoneId, std::shared_ptr<pragma::animation::Bone>> Lua::Bone::GetChildren(lua::State *l, pragma::animation::Bone &bone)
{
	auto t = luabind::newtable(l);
	for(auto &pair : bone.children)
		t[pair.first] = pair.second;
	return t;
}

std::shared_ptr<pragma::animation::Bone> Lua::Bone::GetParent(lua::State *l, pragma::animation::Bone &bone) { return bone.parent.lock(); }
void Lua::Bone::SetName(lua::State *l, pragma::animation::Bone &bone, const std::string &name) { bone.name = name; }
void Lua::Bone::SetParent(lua::State *l, pragma::animation::Bone &bone, pragma::animation::Bone &parent)
{
	ClearParent(l, bone);
	bone.parent = parent.shared_from_this();
	parent.children[bone.ID] = bone.shared_from_this();
}
void Lua::Bone::ClearParent(lua::State *l, pragma::animation::Bone &bone)
{
	if(bone.parent.expired() == false) {
		auto prevParent = bone.parent.lock();
		auto it = prevParent->children.find(bone.ID);
		if(it != prevParent->children.end())
			prevParent->children.erase(it);
	}
	bone.parent = {};
}
