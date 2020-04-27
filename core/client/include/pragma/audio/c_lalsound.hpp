/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_ALSOUND_HPP__
#define __C_ALSOUND_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/classes/lalsound.h>
#include <alsound_buffer.hpp>

namespace Lua
{
	namespace ALSound
	{
		namespace Client
		{
			DLLCLIENT void register_class(luabind::class_<::ALSound> &classDef);
			DLLCLIENT void register_buffer(luabind::class_<al::SoundBuffer> &classDef);
		};
	};
};

#endif
