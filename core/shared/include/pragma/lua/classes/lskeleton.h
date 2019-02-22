#ifndef __LSKELETON_H__
#define __LSKELETON_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/model/animation/skeleton.h"

namespace Lua
{
	namespace Skeleton
	{
		DLLNETWORK void GetRootBones(lua_State *l,::Skeleton &skeleton);
		DLLNETWORK void GetBones(lua_State *l,::Skeleton &skeleton);
		DLLNETWORK void GetBone(lua_State *l,::Skeleton &skeleton,uint32_t boneId);
		DLLNETWORK void LookupBone(lua_State *l,::Skeleton &skeleton,const std::string &name);

		DLLNETWORK void AddBone(lua_State *l,::Skeleton &skeleton,const std::string &name);
		DLLNETWORK void AddBone(lua_State *l,::Skeleton &skeleton,const std::string &name,std::shared_ptr<::Bone> &parent);
		DLLNETWORK void GetBoneCount(lua_State *l,::Skeleton &skeleton);
		DLLNETWORK void Merge(lua_State *l,::Skeleton &skeleton,::Skeleton &skeletonOther);
		DLLNETWORK void ClearBones(lua_State *l,::Skeleton &skeleton);
		DLLNETWORK void MakeRootBone(lua_State *l,::Skeleton &skeleton,std::shared_ptr<::Bone> &bone);
	};
	namespace Bone
	{
		DLLNETWORK void GetName(lua_State *l,std::shared_ptr<::Bone> &bone);
		DLLNETWORK void GetID(lua_State *l,std::shared_ptr<::Bone> &bone);
		DLLNETWORK void GetChildren(lua_State *l,std::shared_ptr<::Bone> &bone);
		DLLNETWORK void GetParent(lua_State *l,std::shared_ptr<::Bone> &bone);
		DLLNETWORK void SetName(lua_State *l,std::shared_ptr<::Bone> &bone,const std::string &name);
		DLLNETWORK void SetParent(lua_State *l,std::shared_ptr<::Bone> &bone,std::shared_ptr<::Bone> &parent);
		DLLNETWORK void ClearParent(lua_State *l,std::shared_ptr<::Bone> &bone);
	};
};

#endif
