#include "stdafx_client.h"
#include "pragma/model/c_model.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include <prosper_command_buffer.hpp>

void Lua::ModelDef::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCModel = luabind::class_<CModelHandle,BaseEntityComponentHandle>("ModelComponent");
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
		Lua::Push<MaterialHandle>(l,mat->GetHandle());
		}));
	entsMod[defCModel];
}
