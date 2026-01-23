// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.entity_components;

export import :entities.components.base;
export import :entities.components.base_sound_emitter;
export import :entities.universal_reference;
export import :scripting.lua.types;

export {
	namespace pragma::LuaCore {
		DLLNETWORK void register_entity_component_classes(lua::State *l, luabind::module_ &mod);
		DLLNETWORK std::optional<Lua::udm_type> get_member_value(lua::State *l, BaseEntityComponent &component, const ComponentMemberInfo &memberInfo);
		DLLNETWORK bool set_member_value(lua::State *l, BaseEntityComponent &component, const ComponentMemberInfo &memberInfo, Lua::udm_type value);
		DLLNETWORK bool set_member_value(lua::State *l, BaseEntityComponent &component, const ComponentMemberInfo &memberInfo, const EntityURef &eref);
		DLLNETWORK bool set_member_value(lua::State *l, BaseEntityComponent &component, const ComponentMemberInfo &memberInfo, const MultiEntityURef &eref);
		DLLNETWORK bool set_member_value(lua::State *l, BaseEntityComponent &component, const ComponentMemberInfo &memberInfo, const EntityUComponentMemberRef &eref);
	};

	namespace Lua::SoundEmitter {
		DLLNETWORK luabind::class_<pragma::BaseSoundEmitterComponent::SoundInfo> RegisterSoundInfo();
	};
};
