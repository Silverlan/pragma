// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

export module pragma.shared:scripting.lua.custom_constructor;

export import pragma.lua;

export namespace pragma::LuaCore {
	template<typename T>
	struct function_traits;

	// Specialization for function pointers
	template<typename R, typename... Args>
	struct function_traits<R (*)(Args...)> {
		using return_type = R;
	};

	// See luabind::detail::construct_aux_helper
	template<typename T, auto TCnstrct, typename... TArgs>
	void custom_constructor(luabind::argument const &self_, TArgs... args)
	{
		luabind::detail::object_rep *self = luabind::touserdata<luabind::detail::object_rep>(self_);
		using TResult = typename function_traits<decltype(TCnstrct)>::return_type;
		if constexpr(!util::is_specialization<TResult, std::shared_ptr>::value) {
			using holder_type = luabind::detail::value_holder<TResult>;

			void *storage = self->allocate(sizeof(holder_type));
			self->set_instance(new(storage) holder_type(nullptr, TCnstrct(std::forward<TArgs>(args)...)));
		}
		else {
			// See luabind::detail::make_pointer_instance
			auto *L = self_.interpreter();
			auto p = TCnstrct(std::forward<TArgs>(args)...);
			using P = TResult;
			auto dynamic = luabind::detail::get_dynamic_class(L, luabind::get_pointer(p));

			auto *cls = luabind::detail::get_pointee_class(L, p, dynamic.first);

			if(!cls) {
				throw std::runtime_error("Trying to use unregistered class: " + std::string(typeid(P).name()));
			}

			using value_type = typename std::remove_reference<P>::type;
			using holder_type = luabind::detail::pointer_holder<value_type>;

			void *storage = self->allocate(sizeof(holder_type));

			try {
				new(storage) holder_type(std::move(p), dynamic.first, dynamic.second);
			}
			catch(...) {
				self->deallocate(storage);
				Lua::Pop(L, 1);
				throw;
			}

			self->set_instance(static_cast<holder_type *>(storage));
		}
	}

	template<typename T, auto TCnstrct, typename... TArgs>
	void define_custom_constructor(lua::State *l)
	{
		auto *registry = luabind::detail::class_registry::get_registry(l);
		auto *crep = registry->find_class(typeid(T));
		assert(crep);
		auto fn = luabind::make_function(l, &custom_constructor<T, TCnstrct, TArgs...>);
		crep->get_table(l);
		auto o = luabind::object {luabind::from_stack(l, -1)};
		luabind::detail::add_overload(o, "__init", fn);
		Lua::Pop(l, 1);
	}
};
