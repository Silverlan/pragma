// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"
#include <string>

export module pragma.shared:scripting.lua.classes.entity_components;

export import :entities.components.base;
export import :entities.components.base_sound_emitter;
export import :entities.universal_reference;
export import :scripting.lua.types;

export {
	namespace pragma::lua {
		DLLNETWORK void register_entity_component_classes(lua_State *l, luabind::module_ &mod);
		DLLNETWORK std::optional<Lua::udm_type> get_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo);
		DLLNETWORK bool set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, Lua::udm_type value);
		DLLNETWORK bool set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, const pragma::EntityURef &eref);
		DLLNETWORK bool set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, const pragma::MultiEntityURef &eref);
		DLLNETWORK bool set_member_value(lua_State *l, pragma::BaseEntityComponent &component, const pragma::ComponentMemberInfo &memberInfo, const pragma::EntityUComponentMemberRef &eref);
	};

	namespace Lua::SoundEmitter {
		DLLNETWORK luabind::class_<pragma::BaseSoundEmitterComponent::SoundInfo> RegisterSoundInfo();
	};
};
