// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.converters.file;

export import :scripting.lua.core;
export import :scripting.lua.libraries.file;
export import pragma.filesystem;

export namespace luabind {
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<ufile::IFile>> : default_converter<ufile::IFile *> {
		using is_native = std::false_type;

		template<class U>
		int match(lua::State *L, U, int index);

		template<class U>
		std::shared_ptr<ufile::IFile> to_cpp(lua::State *L, U u, int index);

		void to_lua(lua::State *L, std::shared_ptr<ufile::IFile> const &p);

		template<class U>
		void converter_postcall(lua::State *, U const &, int)
		{
		}
	};
};

export namespace luabind {
	template<class U>
	int default_converter<std::shared_ptr<ufile::IFile>, void>::match(lua::State *L, U, int index)
	{
		return Lua::IsType<LFile>(L, index) ? 0 : no_match;
	}

	template<class U>
	std::shared_ptr<ufile::IFile> default_converter<std::shared_ptr<ufile::IFile>>::to_cpp(lua::State *L, U u, int index)
	{
		auto *f = luabind::object_cast<LFile *>(object {from_stack(L, index)});
		return f->GetHandle();
	}
}
