// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"



export module pragma.server.entities.components.camera;

import pragma.server.entities;
import pragma.server.entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SCameraComponent final : public BaseEnvCameraComponent {
		public:
			SCameraComponent(BaseEntity &ent) : BaseEnvCameraComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		protected:
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		};
	};

	class DLLSERVER EnvCamera : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
