/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/lua/classes/lmaterial.h"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include <pragma/lua/policies/default_parameter_policy.hpp>
#include <material_manager2.hpp>
#include "luasystem.h"
#include "material.h"
#include <detail_mode.hpp>
#include <luabind/copy_policy.hpp>
#include <sharedutils/alpha_mode.hpp>
#include <sharedutils/util_shaderinfo.hpp>
#include <udm.hpp>

void Lua::Material::register_class(luabind::class_<::Material> &classDef)
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
	classDef.def("IsValid", &::Material::IsValid);
	classDef.def("GetShaderName", &::Material::GetShaderIdentifier);
	classDef.def("GetName", &::Material::GetName);
	classDef.def("GetIndex", &::Material::GetIndex);
	classDef.def("GetPropertyDataBlock", &::Material::GetPropertyDataBlock);
	classDef.def("SetLoaded", &::Material::SetLoaded);
	classDef.def("MergePropertyDataBlock", +[](lua_State *l, ::Material &mat, ::udm::LinkedPropertyWrapper &data) -> bool { return msys::udm_to_data_block(data, *mat.GetPropertyDataBlock()); });
	classDef.def("Copy", static_cast<void (*)(lua_State *, ::Material &)>([](lua_State *l, ::Material &mat) {
		auto matCopy = mat.Copy();
		if(matCopy == nullptr)
			return;
		Lua::Push<msys::MaterialHandle>(l, matCopy);
	}));
	classDef.def("UpdateTextures", &::Material::UpdateTextures);
	classDef.def("UpdateTextures", &::Material::UpdateTextures, luabind::default_parameter_policy<2, bool {false}> {});
	classDef.def("Save", static_cast<luabind::variant<std::string, bool> (*)(lua_State *, ::Material &, udm::AssetData &)>([](lua_State *l, ::Material &mat, udm::AssetData &assetData) -> luabind::variant<std::string, bool> {
		std::string err;
		auto result = mat.Save(assetData, err);
		if(result == false)
			return luabind::object {l, err};
		return luabind::object {l, result};
	}));
	classDef.def("Save", static_cast<luabind::variant<std::string, bool> (*)(lua_State *, ::Material &)>([](lua_State *l, ::Material &mat) -> luabind::variant<std::string, bool> {
		std::string err;
		auto result = mat.Save(err);
		if(result == false)
			return luabind::object {l, err};
		return luabind::object {l, result};
	}));
	classDef.def(
	  "Save", +[](lua_State *l, ::Engine *engine, ::Material &mat, const std::string &fname) -> luabind::variant<std::string, bool> {
		  std::string err;
		  auto result = mat.Save(fname, err);
		  if(result == false)
			  return luabind::object {l, err};
		  engine->PollResourceWatchers();
		  return luabind::object {l, result};
	  });
	classDef.def("IsError", &::Material::IsError);
	classDef.def("IsLoaded", &::Material::IsLoaded);
	classDef.def("IsTranslucent", &::Material::IsTranslucent);
	classDef.def("GetAlphaMode", &::Material::GetAlphaMode);
	classDef.def("GetAlphaCutoff", &::Material::GetAlphaCutoff);
	classDef.def("Reset", &::Material::Reset);
}
