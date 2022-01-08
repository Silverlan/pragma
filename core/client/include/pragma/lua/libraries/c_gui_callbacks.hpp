/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_GUI_CALLBACKS_HPP__
#define __C_GUI_CALLBACKS_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

class WIBase;
namespace Lua
{
	namespace gui
	{
		DLLCLIENT void register_lua_callback(
			std::string className,std::string callbackName,
			const std::function<CallbackHandle(::WIBase&,lua_State*,const std::function<void(const std::function<void()>&)>&)> &fCb
		);
	};
};

#endif
