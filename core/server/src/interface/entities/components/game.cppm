// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.server.entities.components.game;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SGameComponent final : public BaseGameComponent {
		public:
			SGameComponent(BaseEntity &ent) : BaseGameComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER SGameEntity : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
