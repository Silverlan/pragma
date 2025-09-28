// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.server.entities.components.triggers.touch;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER STouchComponent final : public BaseTouchComponent {
		public:
			STouchComponent(BaseEntity &ent) : BaseTouchComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER TriggerTouch : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
