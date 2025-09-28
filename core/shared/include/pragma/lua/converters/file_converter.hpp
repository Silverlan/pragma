// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_FILE_CONVERTER_HPP__
#define __LUA_FILE_CONVERTER_HPP__

#include "pragma/networkdefinitions.h"
#include <luabind/detail/conversion_policies/conversion_policies.hpp>
#include <memory>
#ifdef __linux__
#include <sharedutils/util_ifile.hpp>
#endif

namespace ufile {
	struct IFile;
};

class LFile;
namespace luabind {
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

#endif
