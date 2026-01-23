// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.prop.dynamic;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SPropDynamicComponent final : public BasePropDynamicComponent {
		  public:
			SPropDynamicComponent(ecs::BaseEntity &ent) : BasePropDynamicComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER PropDynamic : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
