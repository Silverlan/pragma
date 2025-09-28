// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.client:entities.components.filter_entity_class;

import :entities.base_entity;

export namespace pragma {
	class DLLCLIENT CFilterClassComponent final : public BaseFilterClassComponent {
	  public:
		CFilterClassComponent(BaseEntity &ent) : BaseFilterClassComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

export class DLLCLIENT CFilterEntityClass : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
