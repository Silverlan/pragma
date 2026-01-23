// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.filter_entity_name;

export import :entities.base_entity;

export namespace pragma {
	class DLLCLIENT CFilterNameComponent final : public BaseFilterNameComponent {
	  public:
		CFilterNameComponent(ecs::BaseEntity &ent) : BaseFilterNameComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};

export class DLLCLIENT CFilterEntityName : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
