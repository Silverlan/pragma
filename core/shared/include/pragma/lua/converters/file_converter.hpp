/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LUA_FILE_CONVERTER_HPP__
#define __LUA_FILE_CONVERTER_HPP__

#include "pragma/networkdefinitions.h"
#include <luabind/detail/conversion_policies/conversion_policies.hpp>
#include <memory>

namespace ufile {struct IFile;};

class LFile;
namespace luabind {
	template<>
	struct DLLNETWORK default_converter<std::shared_ptr<ufile::IFile> >
		: default_converter<ufile::IFile*>
	{
		using is_native = std::false_type;

		template <class U>
		int match(lua_State* L, U, int index);

		template <class U>
		std::shared_ptr<ufile::IFile> to_cpp(lua_State* L, U u, int index);

		void to_lua(lua_State* L, std::shared_ptr<ufile::IFile> const& p);

		template <class U>
		void converter_postcall(lua_State*, U const&, int)
		{}
	};
};

#endif
