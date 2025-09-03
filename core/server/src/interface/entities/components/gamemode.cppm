// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include <pragma/entities/components/base_gamemode_component.hpp>
#include "pragma/entities/s_baseentity.h"

export module pragma.server.entities.components.gamemode;

export {
	namespace pragma {
		class DLLSERVER SGamemodeComponent final : public BaseGamemodeComponent {
		public:
			SGamemodeComponent(BaseEntity &ent) : BaseGamemodeComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER SGamemode : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
