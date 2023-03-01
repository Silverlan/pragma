/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LSOUND_H__
#define __C_LSOUND_H__

#include "pragma/clientdefinitions.h"
#include "pragma/lua/c_ldefinitions.h"
#include "pragma/lua/libraries/lsound.h"

namespace Lua {
	namespace sound {
		DLLCLIENT int register_aux_effect(lua_State *l);
		DLLCLIENT int get_aux_effect(lua_State *l);
		DLLCLIENT int set_distance_model(lua_State *l);
		DLLCLIENT int get_distance_model(lua_State *l);
		DLLCLIENT int is_supported(lua_State *l);

		DLLCLIENT int get_doppler_factor(lua_State *l);
		DLLCLIENT int set_doppler_factor(lua_State *l);
		DLLCLIENT int get_speed_of_sound(lua_State *l);
		DLLCLIENT int set_speed_of_sound(lua_State *l);

		DLLCLIENT int get_device_name(lua_State *l);
		DLLCLIENT int add_global_effect(lua_State *l);
		DLLCLIENT int remove_global_effect(lua_State *l);
		DLLCLIENT int set_global_effect_parameters(lua_State *l);
	};
};

#endif
