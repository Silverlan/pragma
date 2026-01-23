// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.shared;

import :scripting.lua.classes.material;

#ifdef __clang__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::material, Material);
#endif

void Lua::Material::register_class(luabind::class_<pragma::material::Material> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.add_static_constant("ALPHA_MODE_OPAQUE", pragma::math::to_integral(AlphaMode::Opaque));
	classDef.add_static_constant("ALPHA_MODE_MASK", pragma::math::to_integral(AlphaMode::Mask));
	classDef.add_static_constant("ALPHA_MODE_BLEND", pragma::math::to_integral(AlphaMode::Blend));
	classDef.add_static_constant("DETAIL_BLEND_MODE_DECAL_MODULATE", pragma::math::to_integral(pragma::material::DetailMode::DecalModulate));
	classDef.add_static_constant("DETAIL_BLEND_MODE_ADDITIVE", pragma::math::to_integral(pragma::material::DetailMode::Additive));
	classDef.add_static_constant("DETAIL_BLEND_MODE_TRANSLUCENT_DETAIL", pragma::math::to_integral(pragma::material::DetailMode::TranslucentDetail));
	classDef.add_static_constant("DETAIL_BLEND_MODE_BLEND_FACTOR_FADE", pragma::math::to_integral(pragma::material::DetailMode::BlendFactorFade));
	classDef.add_static_constant("DETAIL_BLEND_MODE_TRANSLUCENT_BASE", pragma::math::to_integral(pragma::material::DetailMode::TranslucentBase));
	classDef.add_static_constant("DETAIL_BLEND_MODE_UNLIT_ADDITIVE", pragma::math::to_integral(pragma::material::DetailMode::UnlitAdditive));
	classDef.add_static_constant("DETAIL_BLEND_MODE_UNLIT_ADDITIVE_THRESHOLD_FADE", pragma::math::to_integral(pragma::material::DetailMode::UnlitAdditiveThresholdFade));
	classDef.add_static_constant("DETAIL_BLEND_MODE_TWO_PATTERN_DECAL_MODULATE", pragma::math::to_integral(pragma::material::DetailMode::TwoPatternDecalModulate));
	classDef.add_static_constant("DETAIL_BLEND_MODE_MULTIPLY", pragma::math::to_integral(pragma::material::DetailMode::Multiply));
	classDef.add_static_constant("DETAIL_BLEND_MODE_BASE_MASK_VIA_DETAIL_ALPHA", pragma::math::to_integral(pragma::material::DetailMode::BaseMaskViaDetailAlpha));
	classDef.add_static_constant("DETAIL_BLEND_MODE_SELF_SHADOWED_BUMPMAP", pragma::math::to_integral(pragma::material::DetailMode::SelfShadowedBumpmap));
	classDef.add_static_constant("DETAIL_BLEND_MODE_SSBUMP_ALBEDO", pragma::math::to_integral(pragma::material::DetailMode::SSBumpAlbedo));
	classDef.add_static_constant("DETAIL_BLEND_MODE_COUNT", pragma::math::to_integral(pragma::material::DetailMode::Count));
	classDef.add_static_constant("DETAIL_BLEND_MODE_INVALID", pragma::math::to_integral(pragma::material::DetailMode::Invalid));
	classDef.scope[luabind::def("detail_blend_mode_to_enum", static_cast<void (*)(lua::State *, const std::string &)>([](lua::State *l, const std::string &name) { PushInt(l, pragma::math::to_integral(pragma::material::to_detail_mode(name))); }))];
	classDef.def("IsValid", &pragma::material::Material::IsValid);
	classDef.def("GetShaderName", &pragma::material::Material::GetShaderIdentifier);
	classDef.def("GetName", &pragma::material::Material::GetName);
	classDef.def("GetIndex", &pragma::material::Material::GetIndex);
	classDef.def("GetPropertyDataBlock", &pragma::material::Material::GetPropertyDataBlock);
	classDef.def("SetLoaded", &pragma::material::Material::SetLoaded);
	classDef.def("MergePropertyDataBlock", +[](lua::State *l, pragma::material::Material &mat, ::udm::LinkedPropertyWrapper &data) -> bool { return pragma::material::udm_to_data_block(data, *mat.GetPropertyDataBlock()); });
	classDef.def("Copy", static_cast<void (*)(lua::State *, pragma::material::Material &)>([](lua::State *l, pragma::material::Material &mat) {
		auto matCopy = mat.Copy();
		if(matCopy == nullptr)
			return;
		Lua::Push<pragma::material::MaterialHandle>(l, matCopy->GetHandle());
	}));
	classDef.def("UpdateTextures", &pragma::material::Material::UpdateTextures);
	classDef.def("UpdateTextures", &pragma::material::Material::UpdateTextures, luabind::default_parameter_policy<2, bool {false}> {});
	classDef.def("Save", static_cast<luabind::variant<std::string, bool> (*)(lua::State *, pragma::material::Material &, ::udm::AssetData &)>([](lua::State *l, pragma::material::Material &mat, ::udm::AssetData &assetData) -> luabind::variant<std::string, bool> {
		std::string err;
		auto result = mat.Save(assetData, err);
		if(result == false)
			return luabind::object {l, err};
		return luabind::object {l, result};
	}));
	classDef.def("Save", static_cast<luabind::variant<std::string, bool> (*)(lua::State *, pragma::material::Material &)>([](lua::State *l, pragma::material::Material &mat) -> luabind::variant<std::string, bool> {
		std::string err;
		auto result = mat.Save(err);
		if(result == false)
			return luabind::object {l, err};
		return luabind::object {l, result};
	}));
	classDef.def(
	  "Save", +[](lua::State *l, pragma::Engine *engine, pragma::material::Material &mat, const std::string &fname) -> luabind::variant<std::string, bool> {
		  std::string err;
		  auto result = mat.Save(fname, err);
		  if(result == false)
			  return luabind::object {l, err};
		  pragma::Engine::Get()->PollResourceWatchers();
		  return luabind::object {l, result};
	  });
	classDef.def("IsError", &pragma::material::Material::IsError);
	classDef.def("IsLoaded", &pragma::material::Material::IsLoaded);
	classDef.def("IsTranslucent", &pragma::material::Material::IsTranslucent);
	classDef.def("GetAlphaMode", &pragma::material::Material::GetAlphaMode);
	classDef.def("GetAlphaCutoff", &pragma::material::Material::GetAlphaCutoff);
	classDef.def("Reset", &pragma::material::Material::Reset);
	classDef.def("SetTextureProperty", &pragma::material::Material::SetTextureProperty);
	classDef.def("ClearProperty", static_cast<void (pragma::material::Material::*)(const std::string_view &, bool)>(&pragma::material::Material::ClearProperty));
	classDef.def("ClearProperty", static_cast<void (pragma::material::Material::*)(const std::string_view &, bool)>(&pragma::material::Material::ClearProperty), luabind::default_parameter_policy<3, bool {true}> {});
	classDef.def(
	  "SetProperty", +[](pragma::material::Material &mat, const std::string_view &key, ::udm::Type type, udm_type value) {
		  ::udm::visit(type, [&mat, &key, &value](auto tag) {
			  using T = typename decltype(tag)::type;
			  if constexpr(pragma::material::is_property_type<T>)
				  mat.SetProperty(key, udm::cast_object<T>(value));
		  });
	  });
	classDef.def(
	  "SetProperty", +[](pragma::material::Material &mat, const std::string_view &key, udm_type value) {
		  auto type = udm::determine_udm_type(value);
		  if(type == ::udm::Type::Invalid)
			  return;
		  ::udm::visit(type, [&mat, &key, &value](auto tag) {
			  using T = typename decltype(tag)::type;
			  if constexpr(pragma::material::is_property_type<T>)
				  mat.SetProperty(key, udm::cast_object<T>(value));
		  });
	  });
	classDef.def(
	  "GetProperty", +[](lua::State *l, pragma::material::Material &mat, const std::string_view &key, ::udm::Type type) -> luabind::object {
		  return ::udm::visit(type, [l, &mat, &key](auto tag) -> luabind::object {
			  using T = typename decltype(tag)::type;
			  if constexpr(pragma::material::is_property_type<T>) {
				  T val;
				  if(mat.GetProperty<T>(key, &val))
					  return luabind::object {l, val};
			  }
			  return nil;
		  });
	  });
	classDef.def(
	  "GetProperty", +[](lua::State *l, pragma::material::Material &mat, const std::string_view &key, ::udm::Type type, udm_type defVal) -> luabind::object {
		  return ::udm::visit(type, [l, &mat, &key, &defVal](auto tag) -> luabind::object {
			  using T = typename decltype(tag)::type;
			  if constexpr(pragma::material::is_property_type<T>) {
				  auto val = luabind::object_cast<T>(defVal);
				  val = mat.GetProperty<T>(key, val);
				  return luabind::object {l, val};
			  }
			  return nil;
		  });
	  });
	classDef.def("GetPropertyValueType", &pragma::material::Material::GetPropertyValueType);
}
