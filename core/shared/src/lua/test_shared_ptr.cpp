/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */
#if 0
#include <memory>


#include <luabind/class.hpp>
#include <luabind/make_function.hpp>
#include <luabind/make_function_signature.hpp>
#include <sharedutils/util_shared_handle.hpp>
#include <iostream>
#pragma optimize("",off)

namespace util
{
	template<typename T>
	struct SharedPtr
	{
		std::shared_ptr<T> ptr;
	};

	template<typename T>
	struct WeakPtr
	{
		std::weak_ptr<T> ptr;
	};
};
namespace util
{
	template <class T>
	TWeakSharedHandle<T const>* get_const_holder(TWeakSharedHandle<T>*)
	{
		return 0;
	}

	template<typename T>
	T* get_pointer(const TWeakSharedHandle<T>& pointer)
	{
		return const_cast<T*>(pointer.get());
	}
};

namespace luabind {

	namespace detail
	{
		template<typename T>
		struct pointer_traits<util::TWeakSharedHandle<T>>
		{
			enum { is_pointer = true };
			using value_type = T;
		};

		struct shared_ptr_test_deleter
		{
			shared_ptr_test_deleter(lua_State* L, int index)
				: life_support(get_main_thread(L), L, index)
			{}

			void operator()(void const*)
			{
				handle().swap(life_support);
			}

			handle life_support;
		};

	} // namespace detail

	template <class T>
	struct default_converter<util::TWeakSharedHandle<T> >
		: default_converter<T*>
	{
		using is_native = std::false_type;

		template <class U>
		int match(lua_State* L, U, int index)
		{
			return default_converter<T*>::match(L, decorate_type_t<T*>(), index);
		}

		template <class U>
		util::TWeakSharedHandle<T> to_cpp(lua_State* L, U, int index)
		{
			T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index);

			if(!raw_ptr) {
				return util::TWeakSharedHandle<T>();
			} else {
				return util::TWeakSharedHandle<T>(raw_ptr, detail::shared_ptr_test_deleter(L, index));
			}
		}

		void to_lua(lua_State* L, util::TWeakSharedHandle<T> const& p)
		{
			detail::value_converter().to_lua(L, p);
		}

		template <class U>
		void converter_postcall(lua_State*, U const&, int)
		{}
	};

	template <class T>
	struct default_converter<util::TWeakSharedHandle<T> const&>
		: default_converter<util::TWeakSharedHandle<T> >
	{};



} // namespace luabind

#include "pragma/networkdefinitions.h"

struct TestStruct
{
	luabind::object o;
};

struct TestStructWrapper
{
	util::TWeakSharedHandle<TestStruct> strct;
};

static void TestStructFunc(const TestStruct &ts)
{
	std::cout<<"TestStructFunc has been called!"<<std::endl;
}
#include <luasystem.h>
namespace Lua
{
	DLLNETWORK void HandleLuaError(lua_State *l);
};
DLLNETWORK void test_shared_ptr(lua_State *l)
{
	auto modTest = luabind::module_(l,"test");

	auto defTest = luabind::class_<TestStruct>("Test");
	defTest.def("TestStructFunc",&TestStructFunc);
	modTest[defTest];
	{
		std::shared_ptr<TestStruct> x {};
		x = std::shared_ptr<TestStruct>{new TestStruct{}};
		std::weak_ptr<TestStruct> wp0 {x};

		luabind::object o{l,x};
		luabind::globals(l)["test_object"] = o;

		Lua::RunString(l,"test_object:TestStructFunc()","test");

		//auto useCount = x.use_count();
		x = {};
		std::cout<<"Valid: "<<!wp0.expired()<<std::endl;
		if(Lua::RunString(l,"test_object:TestStructFunc()","test") != Lua::StatusCode::Ok)
			Lua::HandleLuaError(l);

		auto *pLua = luabind::object_cast<TestStruct*>(o);
		auto *pLua2 = luabind::object_cast<std::shared_ptr<TestStruct>*>(o);
		std::cout<<"Pointers: "<<pLua<<","<<pLua2<<std::endl;
	}
	{
		util::TSharedHandle<TestStruct> x {new TestStruct{}};
		util::TWeakSharedHandle<TestStruct> wp0 {x};

		luabind::object o{l,wp0};
		luabind::globals(l)["test_object"] = o;

		Lua::RunString(l,"test_object:TestStructFunc()","test");

		x = {};
		std::cout<<"Valid: "<<!wp0.expired()<<std::endl;

		if(Lua::RunString(l,"test_object:TestStructFunc()","test") != Lua::StatusCode::Ok) // Should throw Lua error, but not crash
			Lua::HandleLuaError(l);

		auto *pLua2 = luabind::object_cast<util::TWeakSharedHandle<TestStruct>*>(o);
		auto *pLua = luabind::object_cast<TestStruct*>(o); // Should crash
		std::cout<<"Pointers: "<<pLua<<","<<pLua2<<std::endl;
	}
}
#pragma optimize("",on)

#endif
