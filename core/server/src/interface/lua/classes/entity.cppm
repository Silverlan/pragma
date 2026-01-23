// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.classes.entity;

import :entities;
export import pragma.lua;

export {
	namespace Lua {
		namespace Entity {
			namespace Server {
				DLLSERVER void register_class(luabind::class_<SBaseEntity, pragma::ecs::BaseEntity> &classDef);
				DLLSERVER void SendNetEvent(lua::State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, ::NetPacket packet, pragma::networking::TargetRecipientFilter &rp);
				DLLSERVER void SendNetEvent(lua::State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, ::NetPacket packet);
				DLLSERVER void SendNetEvent(lua::State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId, pragma::networking::TargetRecipientFilter &rp);
				DLLSERVER void SendNetEvent(lua::State *l, SBaseEntity &ent, nwm::Protocol protocol, unsigned int eventId);
			};
		};
	};
};
