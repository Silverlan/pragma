/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/model/model.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/model/c_modelmesh.h"
#include <prosper_command_buffer.hpp>

void Lua::Animated::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCAnimated = luabind::class_<CAnimatedHandle,BaseEntityComponentHandle>("AnimatedComponent");
	Lua::register_base_animated_component_methods<luabind::class_<CAnimatedHandle,BaseEntityComponentHandle>,CAnimatedHandle>(l,defCAnimated);
	defCAnimated.def("GetEffectiveBoneTransform",static_cast<void(*)(lua_State*,CAnimatedHandle&,uint32_t)>([](lua_State *l,CAnimatedHandle &hAnim,uint32_t boneIdx) {
		pragma::Lua::check_component(l,hAnim);
		auto &transforms = hAnim->GetProcessedBones();
		if(boneIdx >= transforms.size())
			return;
		Lua::Push<umath::ScaledTransform*>(l,&transforms.at(boneIdx));
		}));
	defCAnimated.def("SetEffectiveBoneTransform",static_cast<void(*)(lua_State*,CAnimatedHandle&,uint32_t,const umath::ScaledTransform&)>([](lua_State *l,CAnimatedHandle &hAnim,uint32_t boneIdx,const umath::ScaledTransform &t) {
		pragma::Lua::check_component(l,hAnim);
		auto &transforms = hAnim->GetProcessedBones();
		if(boneIdx >= transforms.size())
			return;
		transforms.at(boneIdx) = t;
		}));
	defCAnimated.def("GetBoneBuffer",static_cast<void(*)(lua_State*,CAnimatedHandle&)>([](lua_State *l,CAnimatedHandle &hAnim) {
		pragma::Lua::check_component(l,hAnim);
		auto *pAnimComponent = hAnim.get();
		if(pAnimComponent == nullptr)
			return;
		auto buf = pAnimComponent->GetBoneBuffer();
		if(buf.expired())
			return;
		Lua::Push(l,buf.lock());
		}));
	defCAnimated.def("GetBoneRenderMatrices",static_cast<void(*)(lua_State*,CAnimatedHandle&)>([](lua_State *l,CAnimatedHandle &hAnim) {
		pragma::Lua::check_component(l,hAnim);
		auto *pAnimComponent = hAnim.get();
		if(pAnimComponent == nullptr)
			return;
		auto &mats = pAnimComponent->GetBoneMatrices();
		auto t = Lua::CreateTable(l);
		auto idx = 1u;
		for(auto &m : mats)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<Mat4>(l,m);
			Lua::SetTableValue(l,t);
		}
		}));
	defCAnimated.def("GetBoneRenderMatrix",static_cast<void(*)(lua_State*,CAnimatedHandle&,uint32_t)>([](lua_State *l,CAnimatedHandle &hAnim,uint32_t boneIndex) {
		pragma::Lua::check_component(l,hAnim);
		auto *pAnimComponent = hAnim.get();
		if(pAnimComponent == nullptr)
			return;
		auto &mats = pAnimComponent->GetBoneMatrices();
		if(boneIndex >= mats.size())
			return;
		auto &m = mats.at(boneIndex);
		Lua::Push<Mat4>(l,m);
		}));
	defCAnimated.def("SetBoneRenderMatrix",static_cast<void(*)(lua_State*,CAnimatedHandle&,uint32_t,const Mat4&)>([](lua_State *l,CAnimatedHandle &hAnim,uint32_t boneIndex,const Mat4 &m) {
		pragma::Lua::check_component(l,hAnim);
		auto *pAnimComponent = hAnim.get();
		if(pAnimComponent == nullptr)
			return;
		auto &mats = pAnimComponent->GetBoneMatrices();
		if(boneIndex >= mats.size())
			return;
		mats.at(boneIndex) = m;
	}));
	defCAnimated.def("GetLocalVertexPosition",static_cast<void(*)(lua_State*,CAnimatedHandle&,std::shared_ptr<::ModelSubMesh>&,uint32_t)>([](lua_State *l,CAnimatedHandle &hAnim,std::shared_ptr<::ModelSubMesh> &subMesh,uint32_t vertexId) {
		pragma::Lua::check_component(l,hAnim);
		Vector3 pos,n;
		auto b = hAnim->GetLocalVertexPosition(static_cast<CModelSubMesh&>(*subMesh),vertexId,pos,n);
		if(b == false)
			return;
		Lua::Push<Vector3>(l,pos);
		}));
	// defCAnimated.add_static_constant("EVENT_ON_SKELETON_UPDATED",pragma::CAnimatedComponent::EVENT_ON_SKELETON_UPDATED);
	// defCAnimated.add_static_constant("EVENT_ON_BONE_MATRICES_UPDATED",pragma::CAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED);
	// defCAnimated.add_static_constant("EVENT_ON_BONE_BUFFER_INITIALIZED",pragma::CAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED);
	entsMod[defCAnimated];
}
