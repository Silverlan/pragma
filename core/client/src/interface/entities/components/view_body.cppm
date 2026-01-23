// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.view_body;

export import :entities.base_entity;

export namespace pragma {
	class DLLCLIENT CViewBodyComponent final : public BaseEntityComponent {
	  public:
		CViewBodyComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};

export class DLLCLIENT CViewBody : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
