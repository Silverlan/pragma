// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "pragma/lua/converters/file_converter.hpp"
#include "pragma/lua/libraries/lfile.h"

void luabind::default_converter<std::shared_ptr<ufile::IFile>>::to_lua(lua_State *L, std::shared_ptr<ufile::IFile> const &p)
{
	auto f = std::make_shared<LFile>();
	f->Construct(p);
	default_converter<std::shared_ptr<LFile>> {}.to_lua(L, f);
}
