/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/model/c_model.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include <prosper_command_buffer.hpp>

void Lua::ModelDef::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCModel = luabind::class_<CModelHandle,BaseEntityComponentHandle>("ModelComponent");
	defCModel.add_static_constant("EVENT_ON_RENDER_MESHES_UPDATED",pragma::CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED);
	Lua::register_base_model_component_methods<luabind::class_<CModelHandle,BaseEntityComponentHandle>,CModelHandle>(l,defCModel);
	defCModel.def("SetMaterialOverride",static_cast<void(*)(lua_State*,CModelHandle&,uint32_t,const std::string&)>([](lua_State *l,CModelHandle &hModel,uint32_t matIdx,const std::string &material) {
		pragma::Lua::check_component(l,hModel);
		hModel->SetMaterialOverride(matIdx,material);
		}));
	defCModel.def("SetMaterialOverride",static_cast<void(*)(lua_State*,CModelHandle&,uint32_t,Material&)>([](lua_State *l,CModelHandle &hModel,uint32_t matIdx,Material &mat) {
		pragma::Lua::check_component(l,hModel);
		hModel->SetMaterialOverride(matIdx,static_cast<CMaterial&>(mat));
		}));
	defCModel.def("ClearMaterialOverride",static_cast<void(*)(lua_State*,CModelHandle&,uint32_t)>([](lua_State *l,CModelHandle &hModel,uint32_t matIdx) {
		pragma::Lua::check_component(l,hModel);
		hModel->ClearMaterialOverride(matIdx);
		}));
	defCModel.def("GetMaterialOverride",static_cast<void(*)(lua_State*,CModelHandle&,uint32_t)>([](lua_State *l,CModelHandle &hModel,uint32_t matIdx) {
		pragma::Lua::check_component(l,hModel);
		auto *mat = hModel->GetMaterialOverride(matIdx);
		if(mat == nullptr)
			return;
		Lua::Push<Material*>(l,mat);
		}));
	defCModel.def("GetRenderMaterial",static_cast<void(*)(lua_State*,CModelHandle&,uint32_t)>([](lua_State *l,CModelHandle &hModel,uint32_t matIdx) {
		pragma::Lua::check_component(l,hModel);
		auto *mat = hModel->GetRenderMaterial(matIdx);
		if(mat == nullptr)
			return;
		Lua::Push<Material*>(l,mat);
		}));
	defCModel.def("GetLOD",static_cast<void(*)(lua_State*,CModelHandle&)>([](lua_State *l,CModelHandle &hModel) {
		pragma::Lua::check_component(l,hModel);
		Lua::PushInt(l,hModel->GetLOD());
	}));
	defCModel.def("SetMaxDrawDistance",static_cast<void(*)(lua_State*,CModelHandle&,float)>([](lua_State *l,CModelHandle &hModel,float maxDrawDist) {
		pragma::Lua::check_component(l,hModel);
		hModel->SetMaxDrawDistance(maxDrawDist);
	}));
	defCModel.def("GetMaxDrawDistance",static_cast<void(*)(lua_State*,CModelHandle&)>([](lua_State *l,CModelHandle &hModel) {
		pragma::Lua::check_component(l,hModel);
		Lua::PushNumber(l,hModel->GetMaxDrawDistance());
	}));
	entsMod[defCModel];
}
