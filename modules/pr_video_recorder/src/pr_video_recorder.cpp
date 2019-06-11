/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "pr_video_recorder.hpp"
#include <luainterface.hpp>
#include <pragma/pragma_module.hpp>

extern "C"
{
	void PRAGMA_EXPORT pragma_initialize_lua(Lua::Interface &l)
	{
		video_recorder::register_lua_library(l);
	}
};
