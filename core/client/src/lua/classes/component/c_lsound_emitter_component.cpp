/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
#include <prosper_command_buffer.hpp>

namespace Lua::SoundEmitter {
	DLLNETWORK luabind::class_<pragma::BaseSoundEmitterComponent::SoundInfo> RegisterSoundInfo();
};
void Lua::SoundEmitter::register_class(lua_State *l, luabind::module_ &entsMod)
{
	auto defCSoundEmitter = pragma::lua::create_entity_component_class<pragma::CSoundEmitterComponent, pragma::BaseSoundEmitterComponent>("SoundEmitterComponent");
	defCSoundEmitter.scope[Lua::SoundEmitter::RegisterSoundInfo()];
	entsMod[defCSoundEmitter];
}
