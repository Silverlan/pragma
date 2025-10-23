// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <memory>

export module pragma.shared:scripting.lua.converters.file;

export import :scripting.lua.core;
export import :scripting.lua.libraries.file;
export import pragma.filesystem;

export namespace luabind {
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<ufile::IFile>> : default_converter<ufile::IFile *> {
		using is_native = std::false_type;

		template<class U>
		int match(lua_State *L, U, int index);

		template<class U>
		std::shared_ptr<ufile::IFile> to_cpp(lua_State *L, U u, int index);

		void to_lua(lua_State *L, std::shared_ptr<ufile::IFile> const &p);

		template<class U>
		void converter_postcall(lua_State *, U const &, int)
		{
		}
	};
};

export {
	template<class U>
	int luabind::default_converter<std::shared_ptr<ufile::IFile>, void>::match(lua_State *L, U, int index)
	{
		return Lua::IsType<LFile>(L, index) ? 0 : no_match;
	}

	template<class U>
	std::shared_ptr<ufile::IFile> luabind::default_converter<std::shared_ptr<ufile::IFile>>::to_cpp(lua_State *L, U u, int index)
	{
		auto *f = luabind::object_cast<LFile *>(luabind::object {luabind::from_stack(L, index)});
		return f->GetHandle();
	}
}
