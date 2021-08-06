/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "pragma/lua/converters/file_converter.hpp"
#include "pragma/lua/libraries/lfile.h"

void luabind::default_converter<std::shared_ptr<VFilePtrInternal> >::to_lua(lua_State* L, std::shared_ptr<VFilePtrInternal> const& p)
{
	auto f = std::make_shared<LFile>();
	f->Construct(p);
	default_converter<std::shared_ptr<LFile>>{}.to_lua(L,f);
}
