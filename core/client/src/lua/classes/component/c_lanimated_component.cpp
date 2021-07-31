/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/model/model.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/lua/policies/optional_policy.hpp"
#include "pragma/model/c_modelmesh.h"
#include <buffers/prosper_swap_buffer.hpp>
#include <prosper_command_buffer.hpp>

void Lua::Animated::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCAnimated = luabind::class_<pragma::CAnimatedComponent,pragma::BaseAnimatedComponent>("AnimatedComponent");
	defCAnimated.def("GetBoneBuffer",static_cast<std::optional<std::shared_ptr<prosper::SwapBuffer>>(*)(lua_State*,pragma::CAnimatedComponent&)>([](lua_State *l,pragma::CAnimatedComponent &hAnim) -> std::optional<std::shared_ptr<prosper::SwapBuffer>> {
		auto buf = hAnim.GetSwapBoneBuffer();
		if(!buf)
			return {};
		return buf->shared_from_this();
	}),luabind::optional_policy<0>{});
	defCAnimated.def("GetBoneRenderMatrices",static_cast<const std::vector<Mat4>&(pragma::CAnimatedComponent::*)() const>(&pragma::CAnimatedComponent::GetBoneMatrices));
	defCAnimated.def("GetBoneRenderMatrix",static_cast<std::optional<Mat4>(*)(lua_State*,pragma::CAnimatedComponent&,uint32_t)>([](lua_State *l,pragma::CAnimatedComponent &hAnim,uint32_t boneIndex) -> std::optional<Mat4> {
		auto &mats = hAnim.GetBoneMatrices();
		if(boneIndex >= mats.size())
			return {};
		return mats.at(boneIndex);
	}),luabind::optional_policy<0>{});
	defCAnimated.def("SetBoneRenderMatrix",static_cast<void(*)(lua_State*,pragma::CAnimatedComponent&,uint32_t,const Mat4&)>([](lua_State *l,pragma::CAnimatedComponent &hAnim,uint32_t boneIndex,const Mat4 &m) {
		auto &mats = hAnim.GetBoneMatrices();
		if(boneIndex >= mats.size())
			return;
		mats.at(boneIndex) = m;
	}));
	defCAnimated.def("GetLocalVertexPosition",static_cast<std::optional<Vector3>(*)(lua_State*,pragma::CAnimatedComponent&,std::shared_ptr<::ModelSubMesh>&,uint32_t)>([](lua_State *l,pragma::CAnimatedComponent &hAnim,std::shared_ptr<::ModelSubMesh> &subMesh,uint32_t vertexId) -> std::optional<Vector3> {
		Vector3 pos,n;
		auto b = hAnim.GetLocalVertexPosition(static_cast<CModelSubMesh&>(*subMesh),vertexId,pos,n);
		if(b == false)
			return {};
		return pos;
	}));
	defCAnimated.def("AreSkeletonUpdateCallbacksEnabled",&pragma::CAnimatedComponent::AreSkeletonUpdateCallbacksEnabled);
	defCAnimated.def("SetSkeletonUpdateCallbacksEnabled",&pragma::CAnimatedComponent::SetSkeletonUpdateCallbacksEnabled);
	defCAnimated.add_static_constant("EVENT_ON_SKELETON_UPDATED",pragma::CAnimatedComponent::EVENT_ON_SKELETON_UPDATED);
	defCAnimated.add_static_constant("EVENT_ON_BONE_MATRICES_UPDATED",pragma::CAnimatedComponent::EVENT_ON_BONE_MATRICES_UPDATED);
	defCAnimated.add_static_constant("EVENT_ON_BONE_BUFFER_INITIALIZED",pragma::CAnimatedComponent::EVENT_ON_BONE_BUFFER_INITIALIZED);
	entsMod[defCAnimated];
}
