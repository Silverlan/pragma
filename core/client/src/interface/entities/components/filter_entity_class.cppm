// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.filter_entity_class;

export import :entities.base_entity;

export namespace pragma {
	class DLLCLIENT CFilterClassComponent final : public BaseFilterClassComponent {
	  public:
		CFilterClassComponent(ecs::BaseEntity &ent) : BaseFilterClassComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};

export class DLLCLIENT CFilterEntityClass : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
