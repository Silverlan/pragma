// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.core;

export import pragma.lua;

export {
	namespace Lua {
		void TypeError(const luabind::object &o, Type type);
		DLLNETWORK Type GetType(const luabind::object &o);
		DLLNETWORK void CheckType(const luabind::object &o, Type type);
	};

	using LuaTableObject = luabind::object;
	using LuaClassObject = luabind::object;
	using LuaFunctionObject = luabind::object;

	class BaseLuaObj;
	namespace Lua {
		DLLNETWORK void PushObject(lua::State *l, BaseLuaObj *o);

		template<class TType>
		bool CheckHandle(lua::State *l, const pragma::util::TSharedHandle<TType> &handle);
		template<class TType>
		bool CheckHandle(lua::State *l, const pragma::util::TWeakSharedHandle<TType> &handle);
		template<class TType>
		bool CheckHandle(lua::State *l, const TType *value);
		template<class TType>
		TType &CheckHandle(lua::State *l, const int32_t idx);
	};

	template<class TType>
	bool Lua::CheckHandle(lua::State *l, const pragma::util::TSharedHandle<TType> &handle)
	{
		if(handle.IsExpired()) {
			PushString(l, "Attempted to use a NULL handle");
			Error(l);
			return false;
		}
		return true;
	}

	template<class TType>
	bool Lua::CheckHandle(lua::State *l, const pragma::util::TWeakSharedHandle<TType> &handle)
	{
		if(handle.IsExpired()) {
			PushString(l, "Attempted to use a NULL handle");
			Error(l);
			return false;
		}
		return true;
	}
	template<class TType>
	TType &Lua::CheckHandle(lua::State *l, const int32_t idx)
	{
		auto *handle = CheckPtr<TType>(l, idx);
		if(handle == nullptr) {
			PushString(l, "Attempted to use a NULL handle");
			Error(l);
			// Unreachable
		}
		return *handle;
	}

	template<class TType>
	bool Lua::CheckHandle(lua::State *l, const TType *value)
	{
		if(value == nullptr) {
			PushString(l, "Attempted to use a NULL handle");
			Error(l);
			// Unreachable
		}
		return true;
	}

	namespace Lua {
		DLLNETWORK StatusCode Execute(lua::State *l, const std::function<StatusCode(int (*traceback)(lua::State *))> &target);
		DLLNETWORK void Execute(lua::State *l, const std::function<void(int (*traceback)(lua::State *), void (*syntaxHandle)(lua::State *, StatusCode))> &target);
		DLLNETWORK void HandleLuaError(lua::State *l);
		DLLNETWORK void HandleLuaError(lua::State *l, StatusCode s);
		DLLNETWORK std::string GetErrorMessagePrefix(lua::State *l);
		template<typename T>
		void table_to_vector(lua::State *l, const luabind::object &t, int32_t tableStackIndex, std::vector<T> &outData);
		template<typename T>
		std::vector<T> table_to_vector(lua::State *l, const luabind::object &t, int32_t tableStackIndex);
		template<typename T>
		luabind::object vector_to_table(lua::State *l, const std::vector<T> &data);

		template<typename T0, typename T1>
		void table_to_map(lua::State *l, const luabind::object &t, int32_t tableStackIndex, std::unordered_map<T0, T1> &outData);
		template<typename T0, typename T1>
		std::unordered_map<T0, T1> table_to_map(lua::State *l, const luabind::object &t, int32_t tableStackIndex);
		template<typename T0, typename T1>
		luabind::object map_to_table(lua::State *l, const std::unordered_map<T0, T1> &data);
	};

	template<typename T>
	void Lua::table_to_vector(lua::State *l, const luabind::object &t, int32_t tableStackIndex, std::vector<T> &outData)
	{
		auto n = GetObjectLength(l, tableStackIndex);
		outData.reserve(outData.size() + n);
		for(auto it = luabind::iterator {t}, end = luabind::iterator {}; it != end; ++it) {
			auto val = luabind::object_cast_nothrow<T>(*it, T {});
			outData.push_back(val);
		}
	}

	template<typename T>
	std::vector<T> Lua::table_to_vector(lua::State *l, const luabind::object &t, int32_t tableStackIndex)
	{
		std::vector<T> result {};
		table_to_vector(l, t, tableStackIndex, result);
		return result;
	}

	template<typename T>
	luabind::object Lua::vector_to_table(lua::State *l, const std::vector<T> &data)
	{
		auto t = luabind::newtable(l);
		uint32_t idx = 1;
		for(auto &v : data)
			t[idx++] = v;
		return t;
	}

	template<typename T0, typename T1>
	void Lua::table_to_map(lua::State *l, const luabind::object &t, int32_t tableStackIndex, std::unordered_map<T0, T1> &outData)
	{
		for(auto it = luabind::iterator {t}, end = luabind::iterator {}; it != end; ++it) {
			auto key = luabind::object_cast_nothrow<T0>(it.key(), T0 {});
			auto val = luabind::object_cast_nothrow<T1>(*it, T1 {});
			outData[key] = val;
		}
	}

	template<typename T0, typename T1>
	std::unordered_map<T0, T1> Lua::table_to_map(lua::State *l, const luabind::object &t, int32_t tableStackIndex)
	{
		std::unordered_map<T0, T1> result {};
		table_to_map(l, t, tableStackIndex, result);
		return result;
	}

	template<typename T0, typename T1>
	luabind::object Lua::map_to_table(lua::State *l, const std::unordered_map<T0, T1> &data)
	{
		auto t = luabind::newtable(l);
		for(auto &pair : data)
			t[pair.first] = pair.second;
		return t;
	}

	namespace Lua {
		template<typename T>
		concept is_trivial_type = std::is_same_v<T, bool> || std::is_arithmetic_v<T> || pragma::util::is_string<T>::value;
		template<typename T, typename = std::enable_if_t<is_trivial_type<T>>>
		T Check(lua::State *l, int32_t n)
		{
			if constexpr(std::is_same_v<T, bool>)
				return CheckBool(l, n);
			else if constexpr(std::is_integral_v<T>)
				return CheckInt(l, n);
			else if constexpr(std::is_arithmetic_v<T>)
				return CheckNumber(l, n);
			else // if constexpr(pragma::util::is_string<T>::value)
				return CheckString(l, n);
		}
		template<typename T, typename = std::enable_if_t<!is_trivial_type<T>>>
		T &Check(lua::State *l, int32_t n)
		{
			CheckUserData(l, n);
			luabind::object o(luabind::from_stack(l, n));
			auto *pValue = luabind::object_cast_nothrow<T *>(o, static_cast<T *>(nullptr));
			if(pValue == nullptr) {
				std::string err = std::string(typeid(T).name()) + " expected, got ";
				err += lua::get_type(l, n);
				lua::error(l, n, err.c_str());
			}
			return *pValue;
		}

		template<typename T, typename = std::enable_if_t<!std::is_arithmetic<T>::value>>
		T *CheckPtr(lua::State *l, int32_t n)
		{
			CheckUserData(l, n);
			luabind::object o(luabind::from_stack(l, n));
			return luabind::object_cast_nothrow<T *>(o, static_cast<T *>(nullptr));
		}

		template<typename T>
		bool IsType(lua::State *l, int32_t n)
		{
			if constexpr(std::is_same_v<T, bool>)
				return IsBool(l, n);
			else if constexpr(std::is_arithmetic_v<T>)
				return IsNumber(l, n);
			else if constexpr(pragma::util::is_string<T>::value)
				return IsString(l, n);
			else {
				if(!IsUserData(l, n))
					return false;
				luabind::object o(luabind::from_stack(l, n));
				auto *pValue = luabind::object_cast_nothrow<T *>(o, static_cast<T *>(nullptr));
				return (pValue != nullptr) ? true : false;
			}
		}
	}
};
