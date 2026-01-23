// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.converters.file;

void luabind::default_converter<std::shared_ptr<ufile::IFile>>::to_lua(lua::State *L, std::shared_ptr<ufile::IFile> const &p)
{
	auto f = pragma::util::make_shared<LFile>();
	f->Construct(p);
	default_converter<std::shared_ptr<LFile>> {}.to_lua(L, f);
}
