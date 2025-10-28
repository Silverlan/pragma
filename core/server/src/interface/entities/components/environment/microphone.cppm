// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.server.entities.components.microphone;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SMicrophoneComponent final : public BaseEnvMicrophoneComponent {
		public:
			SMicrophoneComponent(pragma::ecs::BaseEntity &ent) : BaseEnvMicrophoneComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER EnvMicrophone : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
