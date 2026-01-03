// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.triggers.remove;

import :entities;

export {
	namespace pragma {
		class DLLSERVER STriggerRemoveComponent final : public BaseTriggerRemoveComponent {
		  public:
			STriggerRemoveComponent(ecs::BaseEntity &ent) : BaseTriggerRemoveComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER TriggerRemove : public SBaseEntity {
	  protected:
	  public:
		virtual void Initialize() override;
	};
};
