/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include <prosper_command_buffer.hpp>

void Lua::SoundEmitter::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCSoundEmitter = luabind::class_<pragma::CSoundEmitterComponent,pragma::BaseSoundEmitterComponent>("SoundEmitterComponent");
	defCSoundEmitter.def("CreateSound",static_cast<void(*)(lua_State*,pragma::CSoundEmitterComponent&,std::string,uint32_t,bool)>(&Lua::SoundEmitter::CreateSound));
	defCSoundEmitter.def("EmitSound",static_cast<void(*)(lua_State*,pragma::CSoundEmitterComponent&,std::string,uint32_t,float,float,bool)>(&Lua::SoundEmitter::EmitSound));
	entsMod[defCSoundEmitter];
}
