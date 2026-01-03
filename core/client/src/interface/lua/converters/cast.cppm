// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.converters.cast;

import :model.mesh;
import :model.model_class;
export import pragma.lua;
import pragma.cmaterialsystem;

export namespace luabind {
	template<typename T>
	    requires(std::is_same_v<pragma::util::base_type<T>, pragma::material::CMaterial>)
	struct default_converter<T> : cast_converter<T, copy_qualifiers_t<T, pragma::material::Material>> {};

	template<typename T>
	    requires(std::is_same_v<pragma::util::base_type<T>, pragma::asset::CModel>)
	struct default_converter<T> : cast_converter<T, copy_qualifiers_t<T, pragma::asset::Model>> {};

	template<typename T>
	    requires(std::is_same_v<pragma::util::base_type<T>, pragma::geometry::CModelMesh>)
	struct default_converter<T> : cast_converter<T, copy_qualifiers_t<T, pragma::geometry::ModelMesh>> {};

	template<typename T>
	    requires(std::is_same_v<pragma::util::base_type<T>, pragma::geometry::CModelSubMesh>)
	struct default_converter<T> : cast_converter<T, copy_qualifiers_t<T, pragma::geometry::ModelSubMesh>> {};
};
