// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.triggers.touch;

import :entities;

export {
	namespace pragma {
		class DLLSERVER STouchComponent final : public BaseTouchComponent {
		  public:
			STouchComponent(ecs::BaseEntity &ent) : BaseTouchComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER TriggerTouch : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
