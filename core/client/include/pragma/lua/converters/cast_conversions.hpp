/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_CAST_CONVERSIONS_HPP__
#define __LUA_CAST_CONVERSIONS_HPP__

#include <pragma/lua/converters/cast_converter.hpp>
#include <type_traits>

class Material;
class CMaterial;
class Model;
class CModel;
class ModelMesh;
class CModelMesh;
class ModelSubMesh;
class CModelSubMesh;
namespace luabind {
	template<typename T>
	using base_type = typename std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;

	template<typename T>
	    requires(std::is_same_v<base_type<T>, CMaterial>)
	struct default_converter<T> : cast_converter<T, copy_qualifiers_t<T, Material>> {};

	template<typename T>
	    requires(std::is_same_v<base_type<T>, CModel>)
	struct default_converter<T> : cast_converter<T, copy_qualifiers_t<T, Model>> {};

	template<typename T>
	    requires(std::is_same_v<base_type<T>, CModelMesh>)
	struct default_converter<T> : cast_converter<T, copy_qualifiers_t<T, ModelMesh>> {};

	template<typename T>
	    requires(std::is_same_v<base_type<T>, CModelSubMesh>)
	struct default_converter<T> : cast_converter<T, copy_qualifiers_t<T, ModelSubMesh>> {};
};

#endif
