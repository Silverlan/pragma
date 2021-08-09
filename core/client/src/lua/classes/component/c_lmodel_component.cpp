/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/model/c_model.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include <pragma/lua/converters/cast_converter_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/asset/util_asset.hpp>
#include <prosper_command_buffer.hpp>
#include <cmaterial.h>

void Lua::ModelDef::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCModel = luabind::class_<pragma::CModelComponent,pragma::BaseModelComponent>("ModelComponent");
	defCModel.add_static_constant("EVENT_ON_RENDER_MESHES_UPDATED",pragma::CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED);
	//Lua::register_base_model_component_methods<luabind::class_<CModelHandle,BaseEntityComponentHandle>,CModelHandle>(l,defCModel);
	defCModel.def("SetMaterialOverride",static_cast<void(pragma::CModelComponent::*)(uint32_t,const std::string&)>(&pragma::CModelComponent::SetMaterialOverride));
	defCModel.def("SetMaterialOverride",static_cast<void(pragma::CModelComponent::*)(uint32_t,CMaterial&)>(&pragma::CModelComponent::SetMaterialOverride));
	defCModel.def("SetMaterialOverride",static_cast<void(*)(lua_State*,pragma::CModelComponent&,const std::string&,const std::string&)>([](lua_State *l,pragma::CModelComponent &hModel,const std::string &matSrc,const std::string &matDst) {
		auto &mdl = hModel.GetModel();
		if(!mdl)
			return;
		auto &mats = mdl->GetMaterials();
		auto it = std::find_if(mats.begin(),mats.end(),[&matSrc](const MaterialHandle &hMat) {
			if(!hMat.IsValid())
				return false;
			return pragma::asset::matches(hMat.get()->GetName(),matSrc,pragma::asset::Type::Material);
		});
		if(it == mats.end())
			return;
		hModel.SetMaterialOverride(it -mats.begin(),matDst);
	}));
	defCModel.def("ClearMaterialOverride",&pragma::CModelComponent::ClearMaterialOverride);
	defCModel.def("GetMaterialOverride",&pragma::CModelComponent::GetMaterialOverride);
	defCModel.def("GetRenderMaterial",&pragma::CModelComponent::GetRenderMaterial);
	defCModel.def("GetLOD",&pragma::CModelComponent::GetLOD);
	defCModel.def("SetMaxDrawDistance",&pragma::CModelComponent::SetMaxDrawDistance);
	defCModel.def("GetMaxDrawDistance",&pragma::CModelComponent::GetMaxDrawDistance);
	entsMod[defCModel];
}
