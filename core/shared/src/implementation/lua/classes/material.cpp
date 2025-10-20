// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "string_view"




#include <luabind/copy_policy.hpp>

module pragma.shared;

import :scripting.lua.classes.material;

void Lua::Material::register_class(luabind::class_<msys::Material> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.add_static_constant("ALPHA_MODE_OPAQUE", umath::to_integral(::AlphaMode::Opaque));
	classDef.add_static_constant("ALPHA_MODE_MASK", umath::to_integral(::AlphaMode::Mask));
	classDef.add_static_constant("ALPHA_MODE_BLEND", umath::to_integral(::AlphaMode::Blend));
	classDef.add_static_constant("DETAIL_BLEND_MODE_DECAL_MODULATE", umath::to_integral(msys::DetailMode::DecalModulate));
	classDef.add_static_constant("DETAIL_BLEND_MODE_ADDITIVE", umath::to_integral(msys::DetailMode::Additive));
	classDef.add_static_constant("DETAIL_BLEND_MODE_TRANSLUCENT_DETAIL", umath::to_integral(msys::DetailMode::TranslucentDetail));
	classDef.add_static_constant("DETAIL_BLEND_MODE_BLEND_FACTOR_FADE", umath::to_integral(msys::DetailMode::BlendFactorFade));
	classDef.add_static_constant("DETAIL_BLEND_MODE_TRANSLUCENT_BASE", umath::to_integral(msys::DetailMode::TranslucentBase));
	classDef.add_static_constant("DETAIL_BLEND_MODE_UNLIT_ADDITIVE", umath::to_integral(msys::DetailMode::UnlitAdditive));
	classDef.add_static_constant("DETAIL_BLEND_MODE_UNLIT_ADDITIVE_THRESHOLD_FADE", umath::to_integral(msys::DetailMode::UnlitAdditiveThresholdFade));
	classDef.add_static_constant("DETAIL_BLEND_MODE_TWO_PATTERN_DECAL_MODULATE", umath::to_integral(msys::DetailMode::TwoPatternDecalModulate));
	classDef.add_static_constant("DETAIL_BLEND_MODE_MULTIPLY", umath::to_integral(msys::DetailMode::Multiply));
	classDef.add_static_constant("DETAIL_BLEND_MODE_BASE_MASK_VIA_DETAIL_ALPHA", umath::to_integral(msys::DetailMode::BaseMaskViaDetailAlpha));
	classDef.add_static_constant("DETAIL_BLEND_MODE_SELF_SHADOWED_BUMPMAP", umath::to_integral(msys::DetailMode::SelfShadowedBumpmap));
	classDef.add_static_constant("DETAIL_BLEND_MODE_SSBUMP_ALBEDO", umath::to_integral(msys::DetailMode::SSBumpAlbedo));
	classDef.add_static_constant("DETAIL_BLEND_MODE_COUNT", umath::to_integral(msys::DetailMode::Count));
	classDef.add_static_constant("DETAIL_BLEND_MODE_INVALID", umath::to_integral(msys::DetailMode::Invalid));
	classDef.scope[luabind::def("detail_blend_mode_to_enum", static_cast<void (*)(lua_State *, const std::string &)>([](lua_State *l, const std::string &name) { Lua::PushInt(l, umath::to_integral(msys::to_detail_mode(name))); }))];
	classDef.def("IsValid", &msys::Material::IsValid);
	classDef.def("GetShaderName", &msys::Material::GetShaderIdentifier);
	classDef.def("GetName", &msys::Material::GetName);
	classDef.def("GetIndex", &msys::Material::GetIndex);
	classDef.def("GetPropertyDataBlock", &msys::Material::GetPropertyDataBlock);
	classDef.def("SetLoaded", &msys::Material::SetLoaded);
	classDef.def("MergePropertyDataBlock", +[](lua_State *l, msys::Material &mat, ::udm::LinkedPropertyWrapper &data) -> bool { return msys::udm_to_data_block(data, *mat.GetPropertyDataBlock()); });
	classDef.def("Copy", static_cast<void (*)(lua_State *, msys::Material &)>([](lua_State *l, msys::Material &mat) {
		auto matCopy = mat.Copy();
		if(matCopy == nullptr)
			return;
		Lua::Push<msys::MaterialHandle>(l, matCopy);
	}));
	classDef.def("UpdateTextures", &msys::Material::UpdateTextures);
	classDef.def("UpdateTextures", &msys::Material::UpdateTextures, luabind::default_parameter_policy<2, bool {false}> {});
	classDef.def("Save", static_cast<luabind::variant<std::string, bool> (*)(lua_State *, msys::Material &, ::udm::AssetData &)>([](lua_State *l, msys::Material &mat, ::udm::AssetData &assetData) -> luabind::variant<std::string, bool> {
		std::string err;
		auto result = mat.Save(assetData, err);
		if(result == false)
			return luabind::object {l, err};
		return luabind::object {l, result};
	}));
	classDef.def("Save", static_cast<luabind::variant<std::string, bool> (*)(lua_State *, msys::Material &)>([](lua_State *l, msys::Material &mat) -> luabind::variant<std::string, bool> {
		std::string err;
		auto result = mat.Save(err);
		if(result == false)
			return luabind::object {l, err};
		return luabind::object {l, result};
	}));
	classDef.def(
	  "Save", +[](lua_State *l, pragma::Engine *engine, msys::Material &mat, const std::string &fname) -> luabind::variant<std::string, bool> {
		  std::string err;
		  auto result = mat.Save(fname, err);
		  if(result == false)
			  return luabind::object {l, err};
		  pragma::Engine::Get()->PollResourceWatchers();
		  return luabind::object {l, result};
	  });
	classDef.def("IsError", &msys::Material::IsError);
	classDef.def("IsLoaded", &msys::Material::IsLoaded);
	classDef.def("IsTranslucent", &msys::Material::IsTranslucent);
	classDef.def("GetAlphaMode", &msys::Material::GetAlphaMode);
	classDef.def("GetAlphaCutoff", &msys::Material::GetAlphaCutoff);
	classDef.def("Reset", &msys::Material::Reset);
	classDef.def("SetTextureProperty", &msys::Material::SetTextureProperty);
	classDef.def("ClearProperty", static_cast<void (msys::Material::*)(const std::string_view &, bool)>(&msys::Material::ClearProperty));
	classDef.def("ClearProperty", static_cast<void (msys::Material::*)(const std::string_view &, bool)>(&msys::Material::ClearProperty), luabind::default_parameter_policy<3, bool {true}> {});
	classDef.def(
	  "SetProperty", +[](msys::Material &mat, const std::string_view &key, ::udm::Type type, Lua::udm_type value) {
		  ::udm::visit(type, [&mat, &key, &value](auto tag) {
			  using T = typename decltype(tag)::type;
			  if constexpr(msys::is_property_type<T>)
				  mat.SetProperty(key, Lua::udm::cast_object<T>(value));
		  });
	  });
	classDef.def(
	  "SetProperty", +[](msys::Material &mat, const std::string_view &key, Lua::udm_type value) {
		  auto type = Lua::udm::determine_udm_type(value);
		  if(type == ::udm::Type::Invalid)
			  return;
		  ::udm::visit(type, [&mat, &key, &value](auto tag) {
			  using T = typename decltype(tag)::type;
			  if constexpr(msys::is_property_type<T>)
				  mat.SetProperty(key, Lua::udm::cast_object<T>(value));
		  });
	  });
	classDef.def(
	  "GetProperty", +[](lua_State *l, msys::Material &mat, const std::string_view &key, ::udm::Type type) -> luabind::object {
		  return ::udm::visit(type, [l, &mat, &key](auto tag) -> luabind::object {
			  using T = typename decltype(tag)::type;
			  if constexpr(msys::is_property_type<T>) {
				  T val;
				  if(mat.GetProperty<T>(key, &val))
					  return luabind::object {l, val};
			  }
			  return Lua::nil;
		  });
	  });
	classDef.def(
	  "GetProperty", +[](lua_State *l, msys::Material &mat, const std::string_view &key, ::udm::Type type, Lua::udm_type defVal) -> luabind::object {
		  return ::udm::visit(type, [l, &mat, &key, &defVal](auto tag) -> luabind::object {
			  using T = typename decltype(tag)::type;
			  if constexpr(msys::is_property_type<T>) {
				  auto val = luabind::object_cast<T>(defVal);
				  val = mat.GetProperty<T>(key, val);
				  return luabind::object {l, val};
			  }
			  return Lua::nil;
		  });
	  });
	classDef.def("GetPropertyValueType", &msys::Material::GetPropertyValueType);
}
