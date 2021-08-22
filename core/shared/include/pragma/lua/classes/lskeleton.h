/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LSKELETON_H__
#define __LSKELETON_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/model/animation/skeleton.h"
#include <panima/types.hpp>

namespace Lua
{
	namespace Skeleton
	{
		DLLNETWORK bool IsRootBone(lua_State *l,panima::Skeleton &skeleton,const std::string &boneId);
		DLLNETWORK bool IsRootBone(lua_State *l,panima::Skeleton &skeleton,uint32_t boneId);
		DLLNETWORK luabind::map<uint32_t,std::shared_ptr<panima::Bone>> GetRootBones(lua_State *l,panima::Skeleton &skeleton);
		DLLNETWORK luabind::tableT<std::shared_ptr<panima::Bone>> GetBones(lua_State *l,panima::Skeleton &skeleton);
		DLLNETWORK std::shared_ptr<panima::Bone> GetBone(lua_State *l,panima::Skeleton &skeleton,uint32_t boneId);
		DLLNETWORK int32_t LookupBone(lua_State *l,panima::Skeleton &skeleton,const std::string &name);

		DLLNETWORK std::shared_ptr<panima::Bone> AddBone(lua_State *l,panima::Skeleton &skeleton,const std::string &name);
		DLLNETWORK std::shared_ptr<panima::Bone> AddBone(lua_State *l,panima::Skeleton &skeleton,const std::string &name,panima::Bone &parent);
		DLLNETWORK void ClearBones(lua_State *l,panima::Skeleton &skeleton);
		DLLNETWORK bool MakeRootBone(lua_State *l,panima::Skeleton &skeleton,panima::Bone &bone);
		DLLNETWORK luabind::map<uint16_t,luabind::tableT<void>> GetBoneHierarchy(lua_State *l,panima::Skeleton &skeleton);
	};
	namespace Bone
	{
		DLLNETWORK std::string GetName(lua_State *l,panima::Bone &bone);
		DLLNETWORK BoneId GetID(lua_State *l,panima::Bone &bone);
		DLLNETWORK luabind::map<BoneId,std::shared_ptr<panima::Bone>> GetChildren(lua_State *l,panima::Bone &bone);
		DLLNETWORK std::shared_ptr<panima::Bone> GetParent(lua_State *l,panima::Bone &bone);
		DLLNETWORK void SetName(lua_State *l,panima::Bone &bone,const std::string &name);
		DLLNETWORK void SetParent(lua_State *l,panima::Bone &bone,panima::Bone &parent);
		DLLNETWORK void ClearParent(lua_State *l,panima::Bone &bone);
	};
};

#endif
