// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.client:entities.components.env_microphone;

import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CMicrophoneComponent final : public BaseEnvMicrophoneComponent {
		public:
			CMicrophoneComponent(BaseEntity &ent) : BaseEnvMicrophoneComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLCLIENT CEnvMicrophone : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
