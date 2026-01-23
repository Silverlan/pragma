// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.func.button;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SButtonComponent final : public BaseFuncButtonComponent {
		  public:
			SButtonComponent(ecs::BaseEntity &ent) : BaseFuncButtonComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER FuncButton : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
