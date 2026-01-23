// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.func_button;

export import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CButtonComponent final : public BaseFuncButtonComponent {
		  public:
			CButtonComponent(ecs::BaseEntity &ent) : BaseFuncButtonComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLCLIENT CFuncButton : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
