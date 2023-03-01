/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include <pragma/lua/converters/cast_converter_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <pragma/lua/policies/default_parameter_policy.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
#include <pragma/lua/custom_constructor.hpp>
#include <pragma/asset/util_asset.hpp>
#include <prosper_command_buffer.hpp>
#include <buffers/prosper_render_buffer.hpp>
#include <luabind/copy_policy.hpp>
#include <cmaterial.h>

void Lua::ModelDef::register_class(lua_State *l, luabind::module_ &entsMod)
{
	auto defCModel = pragma::lua::create_entity_component_class<pragma::CModelComponent, pragma::BaseModelComponent>("ModelComponent");
	defCModel.add_static_constant("EVENT_ON_RENDER_MESHES_UPDATED", pragma::CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED);
	defCModel.add_static_constant("EVENT_ON_MATERIAL_OVERRIDES_CLEARED", pragma::CModelComponent::EVENT_ON_MATERIAL_OVERRIDES_CLEARED);
	//Lua::register_base_model_component_methods<luabind::class_<CModelHandle,BaseEntityComponentHandle>,CModelHandle>(l,defCModel);
	defCModel.def("SetMaterialOverride", static_cast<void (pragma::CModelComponent::*)(uint32_t, const std::string &)>(&pragma::CModelComponent::SetMaterialOverride));
	defCModel.def("SetMaterialOverride", static_cast<void (pragma::CModelComponent::*)(uint32_t, CMaterial &)>(&pragma::CModelComponent::SetMaterialOverride));
	defCModel.def(
	  "SetMaterialOverride", +[](lua_State *l, pragma::CModelComponent &hModel, const std::string &matSrc, const std::string &matDst) {
		  auto &mdl = hModel.GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const msys::MaterialHandle &hMat) {
			  if(!hMat)
				  return false;
			  return pragma::asset::matches(hMat.get()->GetName(), matSrc, pragma::asset::Type::Material);
		  });
		  if(it == mats.end())
			  return;
		  hModel.SetMaterialOverride(it - mats.begin(), matDst);
	  });
	defCModel.def(
	  "SetMaterialOverride", +[](lua_State *l, pragma::CModelComponent &hModel, const std::string &matSrc, CMaterial &matDst) {
		  auto &mdl = hModel.GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const msys::MaterialHandle &hMat) {
			  if(!hMat)
				  return false;
			  return pragma::asset::matches(hMat.get()->GetName(), matSrc, pragma::asset::Type::Material);
		  });
		  if(it == mats.end())
			  return;
		  hModel.SetMaterialOverride(it - mats.begin(), matDst);
	  });
	defCModel.def(
	  "ClearMaterialOverride", +[](lua_State *l, pragma::CModelComponent &hModel, const std::string &matSrc) {
		  auto &mdl = hModel.GetModel();
		  if(!mdl)
			  return;
		  auto &mats = mdl->GetMaterials();
		  auto it = std::find_if(mats.begin(), mats.end(), [&matSrc](const msys::MaterialHandle &hMat) {
			  if(!hMat)
				  return false;
			  return pragma::asset::matches(hMat.get()->GetName(), matSrc, pragma::asset::Type::Material);
		  });
		  if(it == mats.end())
			  return;
		  hModel.ClearMaterialOverride(it - mats.begin());
	  });
	defCModel.def("ClearMaterialOverride", &pragma::CModelComponent::ClearMaterialOverride);
	defCModel.def("ClearMaterialOverrides", &pragma::CModelComponent::ClearMaterialOverrides);
	defCModel.def("GetMaterialOverride", &pragma::CModelComponent::GetMaterialOverride);
	defCModel.def("GetRenderMaterial", static_cast<CMaterial *(pragma::CModelComponent::*)(uint32_t, uint32_t) const>(&pragma::CModelComponent::GetRenderMaterial));
	defCModel.def("GetRenderMaterial", static_cast<CMaterial *(pragma::CModelComponent::*)(uint32_t) const>(&pragma::CModelComponent::GetRenderMaterial));
	defCModel.def("GetLOD", &pragma::CModelComponent::GetLOD);
	defCModel.def("IsAutoLodEnabled", &pragma::CModelComponent::IsAutoLodEnabled);
	defCModel.def("SetAutoLodEnabled", &pragma::CModelComponent::SetAutoLodEnabled);
	defCModel.def("SetMaxDrawDistance", &pragma::CModelComponent::SetMaxDrawDistance);
	defCModel.def("GetMaxDrawDistance", &pragma::CModelComponent::GetMaxDrawDistance);
	defCModel.def("UpdateRenderMeshes", &pragma::CModelComponent::UpdateRenderMeshes);
	defCModel.def("SetRenderMeshesDirty", &pragma::CModelComponent::SetRenderMeshesDirty);
	defCModel.def("ReloadRenderBufferList", &pragma::CModelComponent::ReloadRenderBufferList, luabind::default_parameter_policy<2, false> {});
	defCModel.def("ReloadRenderBufferList", &pragma::CModelComponent::ReloadRenderBufferList);
	defCModel.def("IsDepthPrepassEnabled", &pragma::CModelComponent::IsDepthPrepassEnabled);
	defCModel.def("SetDepthPrepassEnabled", &pragma::CModelComponent::SetDepthPrepassEnabled);
	defCModel.def("SetRenderBufferData", &pragma::CModelComponent::SetRenderBufferData);
	defCModel.def(
	  "GetRenderBufferData", +[](pragma::CModelComponent &c) -> std::vector<pragma::rendering::RenderBufferData> { return c.GetRenderBufferData(); });
	defCModel.def("AddRenderMesh", &pragma::CModelComponent::AddRenderMesh);
	defCModel.def("AddRenderMesh", &pragma::CModelComponent::AddRenderMesh, luabind::default_parameter_policy<4, true> {});
	defCModel.def(
	  "GetRenderMeshes", +[](pragma::CModelComponent &c) -> std::vector<std::shared_ptr<ModelSubMesh>> { return c.GetRenderMeshes(); });

	auto defRenderBufferData = luabind::class_<pragma::rendering::RenderBufferData>("RenderBufferData");
	defRenderBufferData.def(luabind::constructor<>());
	defRenderBufferData.def_readwrite("enableDepthPrepass", &pragma::rendering::RenderBufferData::enableDepthPrepass);
	defRenderBufferData.def_readwrite("pipelineSpecializationFlags", &pragma::rendering::RenderBufferData::pipelineSpecializationFlags);
	defRenderBufferData.def_readwrite("material", &pragma::rendering::RenderBufferData::material);
	defRenderBufferData.def_readwrite("renderBuffer", &pragma::rendering::RenderBufferData::renderBuffer);
	defCModel.scope[defRenderBufferData];

	entsMod[defCModel];

	pragma::lua::define_custom_constructor<pragma::rendering::RenderBufferData, [](const pragma::rendering::RenderBufferData &renderBufferData) -> pragma::rendering::RenderBufferData { return renderBufferData; }, const pragma::rendering::RenderBufferData &>(l);
}
