// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.func_portal;

export import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CFuncPortalComponent final : public BaseFuncPortalComponent {
		  public:
			CFuncPortalComponent(ecs::BaseEntity &ent) : BaseFuncPortalComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLCLIENT CFuncPortal : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
