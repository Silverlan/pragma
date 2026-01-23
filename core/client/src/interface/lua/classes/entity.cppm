// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.classes.entity;

export import :entities.base_entity;

export namespace Lua {
	namespace Entity {
		namespace Client {
			DLLCLIENT void register_class(luabind::class_<pragma::ecs::CBaseEntity, pragma::ecs::BaseEntity> &classDef);
			DLLCLIENT void GetEffectiveBoneTransform(lua::State *l, pragma::ecs::CBaseEntity &ent, uint32_t boneIdx);
			DLLCLIENT void SendNetEvent(lua::State *l, pragma::ecs::CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, const ::NetPacket &packet);
			DLLCLIENT void SendNetEvent(lua::State *l, pragma::ecs::CBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId);
		};
	};
};
