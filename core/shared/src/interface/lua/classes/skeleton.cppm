// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.skeleton;

export import :model.animation.skeleton;
export import :scripting.lua.api;
import panima;

export namespace Lua {
	namespace Skeleton {
		DLLNETWORK bool IsRootBone(lua::State *l, pragma::animation::Skeleton &skeleton, const std::string &boneId);
		DLLNETWORK bool IsRootBone(lua::State *l, pragma::animation::Skeleton &skeleton, uint32_t boneId);
		DLLNETWORK luabind::map<uint32_t, std::shared_ptr<pragma::animation::Bone>> GetRootBones(lua::State *l, pragma::animation::Skeleton &skeleton);
		DLLNETWORK luabind::tableT<std::shared_ptr<pragma::animation::Bone>> GetBones(lua::State *l, pragma::animation::Skeleton &skeleton);
		DLLNETWORK std::shared_ptr<pragma::animation::Bone> GetBone(lua::State *l, pragma::animation::Skeleton &skeleton, uint32_t boneId);
		DLLNETWORK int32_t LookupBone(lua::State *l, pragma::animation::Skeleton &skeleton, const std::string &name);

		DLLNETWORK std::shared_ptr<pragma::animation::Bone> AddBone(lua::State *l, pragma::animation::Skeleton &skeleton, const std::string &name);
		DLLNETWORK std::shared_ptr<pragma::animation::Bone> AddBone(lua::State *l, pragma::animation::Skeleton &skeleton, const std::string &name, pragma::animation::Bone &parent);
		DLLNETWORK void ClearBones(lua::State *l, pragma::animation::Skeleton &skeleton);
		DLLNETWORK bool MakeRootBone(lua::State *l, pragma::animation::Skeleton &skeleton, pragma::animation::Bone &bone);
		DLLNETWORK luabind::map<uint16_t, luabind::tableT<void>> GetBoneHierarchy(lua::State *l, pragma::animation::Skeleton &skeleton);
	};
	namespace Bone {
		DLLNETWORK std::string GetName(lua::State *l, pragma::animation::Bone &bone);
		DLLNETWORK pragma::animation::BoneId GetID(lua::State *l, pragma::animation::Bone &bone);
		DLLNETWORK luabind::map<pragma::animation::BoneId, std::shared_ptr<pragma::animation::Bone>> GetChildren(lua::State *l, pragma::animation::Bone &bone);
		DLLNETWORK std::shared_ptr<pragma::animation::Bone> GetParent(lua::State *l, pragma::animation::Bone &bone);
		DLLNETWORK void SetName(lua::State *l, pragma::animation::Bone &bone, const std::string &name);
		DLLNETWORK void SetParent(lua::State *l, pragma::animation::Bone &bone, pragma::animation::Bone &parent);
		DLLNETWORK void ClearParent(lua::State *l, pragma::animation::Bone &bone);
	};
};
