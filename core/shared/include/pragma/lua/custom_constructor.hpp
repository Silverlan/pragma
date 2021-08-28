/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __CUSTOM_CONSTRUCTOR_HPP__
#define __CUSTOM_CONSTRUCTOR_HPP__

#include <luabind/lua_argument_proxy.hpp>
#include <luabind/detail/object_rep.hpp>
#include <luabind/function.hpp>

namespace pragma::lua
{
	// See luabind::detail::construct_aux_helper
	template<typename T,auto TCnstrct,typename ...TArgs>
	static void custom_constructor(luabind::argument const &self_, TArgs... args)
	{
		using holder_type = luabind::detail::value_holder<T>;
		luabind::detail::object_rep* self = luabind::touserdata<luabind::detail::object_rep>(self_);

		void* storage = self->allocate(sizeof(holder_type));
		self->set_instance(new (storage) holder_type(nullptr,TCnstrct(std::forward<TArgs>(args)...)));
	}

	template<typename T,auto TCnstrct,typename ...TArgs>
		static void define_custom_constructor(lua_State *l)
	{
		auto *registry = luabind::detail::class_registry::get_registry(l);
		auto *crep = registry->find_class(typeid(T));
		assert(crep);
		auto fn = luabind::make_function(l,&custom_constructor<T,TCnstrct,TArgs...>);
		crep->get_table(l);
		auto o = luabind::object{luabind::from_stack(l,-1)};
		luabind::detail::add_overload(o,"__init",fn);
		lua_pop(l,1);
	}
};

#endif
