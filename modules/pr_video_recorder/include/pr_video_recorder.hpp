/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __PR_VIDEO_RECORDER_HPP__
#define __PR_VIDEO_RECORDER_HPP__

namespace Lua {class Interface;};
namespace video_recorder
{
	void register_lua_library(Lua::Interface &l);
};

#endif
