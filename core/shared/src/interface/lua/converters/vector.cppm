// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

export module pragma.shared:scripting.lua.converters.vector;

export import pragma.lua;

export namespace luabind {
	template<typename T>
	struct default_converter<std::vector<T>> : native_converter_base<std::vector<T>> {
		enum { consumed_args = 1 };

		template<typename U>
		std::vector<T> to_cpp(lua::State *L, U u, int index);

		template<class U>
		static int match(lua::State *l, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}

		void to_lua(lua::State *L, std::vector<T> const &x);
		void to_lua(lua::State *L, std::vector<T> *x);
	};

	template<typename T>
	struct default_converter<const std::vector<T>> : default_converter<std::vector<T>> {};

	template<typename T>
	struct default_converter<std::vector<T> const &> : default_converter<std::vector<T>> {};

	template<typename T>
	struct default_converter<std::vector<T> &&> : default_converter<std::vector<T>> {};
}

export namespace luabind {
	template<class TMap>
	struct map_converter : native_converter_base<TMap> {
		using T0 = typename TMap::key_type;
		using T1 = typename TMap::value_type::second_type;
		enum { consumed_args = 1 };

		template<typename U>
		TMap to_cpp(lua::State *L, U u, int index);

		template<class U>
		static int match(lua::State *l, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}

		void to_lua(lua::State *L, TMap const &x);
		void to_lua(lua::State *L, TMap *x);
	};

	// std::map
	template<typename T0, typename T1>
	struct default_converter<std::map<T0, T1>> : map_converter<std::map<T0, T1>> {};

	template<typename T0, typename T1>
	struct default_converter<const std::map<T0, T1>> : default_converter<std::map<T0, T1>> {};

	template<typename T0, typename T1>
	struct default_converter<std::map<T0, T1> const &> : default_converter<std::map<T0, T1>> {};

	template<typename T0, typename T1>
	struct default_converter<std::map<T0, T1> &&> : default_converter<std::map<T0, T1>> {};

	// std::unordered_map
	template<typename T0, typename T1>
	struct default_converter<std::unordered_map<T0, T1>> : map_converter<std::unordered_map<T0, T1>> {};

	template<typename T0, typename T1>
	struct default_converter<const std::unordered_map<T0, T1>> : default_converter<std::unordered_map<T0, T1>> {};

	template<typename T0, typename T1>
	struct default_converter<std::unordered_map<T0, T1> const &> : default_converter<std::unordered_map<T0, T1>> {};

	template<typename T0, typename T1>
	struct default_converter<std::unordered_map<T0, T1> &&> : default_converter<std::unordered_map<T0, T1>> {};
}

export namespace luabind {
	template<typename T, size_t SIZE>
	struct default_converter<std::array<T, SIZE>> : native_converter_base<std::array<T, SIZE>> {
		enum { consumed_args = 1 };

		template<typename U>
		std::array<T, SIZE> to_cpp(lua::State *L, U u, int index);
		template<class U>
		static int match(lua::State *l, U u, int index);

		template<class U>
		void converter_postcall(lua::State *, U u, int)
		{
		}

		void to_lua(lua::State *L, std::array<T, SIZE> const &x);
		void to_lua(lua::State *L, std::array<T, SIZE> *x);
	};

	template<typename T, size_t SIZE>
	struct default_converter<const std::array<T, SIZE>> : default_converter<std::array<T, SIZE>> {};

	template<typename T, size_t SIZE>
	struct default_converter<std::array<T, SIZE> const &> : default_converter<std::array<T, SIZE>> {};

	template<typename T, size_t SIZE>
	struct default_converter<std::array<T, SIZE> &&> : default_converter<std::array<T, SIZE>> {};
}

export {
	namespace luabind {
		template<class T>
		struct is_unordered_map {
			static constexpr bool value = false;
		};

		template<class Key, class Value>
		struct is_unordered_map<std::unordered_map<Key, Value>> {
			static constexpr bool value = true;
		};

		template<typename T>
		template<typename U>
		std::vector<T> default_converter<std::vector<T>>::to_cpp(lua::State *L, U u, int index)
		{
			default_converter<T> converter;

			auto o = object {from_stack(L, index)};
			auto n = Lua::GetObjectLength(L, o);
			std::vector<T> v;
			v.reserve(n);
			for(iterator it {o}, end; it != end; ++it) {
				object o = *it;
				o.push(L);
				if(converter.match(L, decorate_type_t<T>(), -1) != no_match)
					v.push_back(converter.to_cpp(L, decorate_type_t<T>(), -1));
				Lua::Pop(L, 1);
			}
			return v;
		}

		template<typename T>
		template<class U>
		int default_converter<std::vector<T>>::match(lua::State *l, U u, int index)
		{
			return Lua::IsTable(l, index) ? 0 : no_match;
		}

		template<typename T>
		void default_converter<std::vector<T>>::to_lua(lua::State *L, std::vector<T> const &x)
		{
			default_converter<T> converter;
			auto t = newtable(L);
			t.push(L);
			int index = 1;

			for(const auto &element : x) {
				if constexpr(std::is_fundamental_v<decltype(element)>)
					t[index] = element;
				else {
					auto top = Lua::GetStackTop(L);
					converter.to_lua(L, element);

					auto num = Lua::GetStackTop(L) - top;
					if(num == 1)
						Lua::SetTableValue(L, -2, index);
					else {
						// Multiple values, add as sub-table
						auto subTable = newtable(L);
						for(int i = 0; i < num; ++i)
							subTable[i + 1] = object {from_stack(L, -num + i)};
						Lua::Pop(L, num);
						subTable.push(L);
						Lua::SetTableValue(L, -2, index); // Add the sub-table to the main table
					}
				}
				++index;
			}
		}

		template<typename T>
		void default_converter<std::vector<T>>::to_lua(lua::State *L, std::vector<T> *x)
		{
			if(!x)
				newtable(L).push(L);
			else
				to_lua(L, *x);
		}

		template<class TMap>
		template<typename U>
		TMap map_converter<TMap>::to_cpp(lua::State *L, U u, int index)
		{
			default_converter<T0> converter0;
			default_converter<T1> converter1;

			auto o = object {from_stack(L, index)};
			auto n = Lua::GetObjectLength(L, o);
			TMap v;
			if constexpr(is_unordered_map<TMap>::value)
				v.reserve(n);
			for(iterator it {o}, end; it != end; ++it) {
				object oKey = it.key();
				object o = *it;

				oKey.push(L);
				o.push(L);
				if(converter0.match(L, decorate_type_t<T0>(), -2) != no_match && converter1.match(L, decorate_type_t<T1>(), -1) != no_match)
					v[converter0.to_cpp(L, decorate_type_t<T0>(), -2)] = converter1.to_cpp(L, decorate_type_t<T1>(), -1);
				Lua::Pop(L, 2);
			}
			return v;
		}

		template<class TMap>
		template<class U>
		int map_converter<TMap>::match(lua::State *l, U u, int index)
		{
			return Lua::IsTable(l, index) ? 0 : no_match;
		}

		template<class TMap>
		void map_converter<TMap>::to_lua(lua::State *L, TMap const &x)
		{
			default_converter<T0> converter0;
			default_converter<T1> converter1;
			auto t = newtable(L);
			t.push(L);
			for(const auto &pair : x) {
				if constexpr(std::is_fundamental_v<decltype(pair.first)>) {
					if constexpr(std::is_fundamental_v<decltype(pair.second)>)
						t[pair.first] = pair.second;
					else {
						converter1.to_lua(L, pair.second);
						t[pair.first] = object {from_stack(L, -1)};
						Lua::Pop(L, 1);
					}
				}
				else {
					converter0.to_lua(L, pair.first);
					converter1.to_lua(L, pair.second);
					Lua::SetRaw(L, -3);
				}
			}
		}

		template<class TMap>
		void map_converter<TMap>::to_lua(lua::State *L, TMap *x)
		{
			if(!x)
				newtable(L).push(L);
			else
				to_lua(L, *x);
		}

		template<typename T, size_t SIZE>
		template<typename U>
		std::array<T, SIZE> default_converter<std::array<T, SIZE>>::to_cpp(lua::State *L, U u, int index)
		{
			default_converter<T> converter;

			auto o = object {from_stack(L, index)};
			auto n = Lua::GetObjectLength(L, o);
			assert(n == SIZE);
			if(n > SIZE)
				n = SIZE;
			std::array<T, SIZE> v;
			uint32_t i = 0;
			for(iterator it {o}, end; it != end; ++it) {
				object o = *it;
				o.push(L);
				if(converter.match(L, decorate_type_t<T>(), -1) != no_match)
					v[i] = converter.to_cpp(L, decorate_type_t<T>(), -1);
				Lua::Pop(L, 1);

				++i;
			}
			return v;
		}

		template<typename T, size_t SIZE>
		template<class U>
		int default_converter<std::array<T, SIZE>>::match(lua::State *l, U u, int index)
		{
			if(!Lua::IsTable(l, index))
				return no_match;
			auto n = Lua::GetObjectLength(l, index);
			if(n != SIZE)
				return no_match;
			return 0;
		}

		template<typename T, size_t SIZE>
		void default_converter<std::array<T, SIZE>>::to_lua(lua::State *L, std::array<T, SIZE> const &x)
		{
			default_converter<T> converter;
			auto t = newtable(L);
			t.push(L);
			int index = 1;

			for(const auto &element : x) {
				if constexpr(std::is_fundamental_v<decltype(element)>)
					t[index] = element;
				else {
					converter.to_lua(L, element);
					Lua::SetTableValue(L, -2, index);
				}
				++index;
			}
		}

		template<typename T, size_t SIZE>
		void default_converter<std::array<T, SIZE>>::to_lua(lua::State *L, std::array<T, SIZE> *x)
		{
			if(!x)
				newtable(L).push(L);
			else
				to_lua(L, *x);
		}
	}
}
