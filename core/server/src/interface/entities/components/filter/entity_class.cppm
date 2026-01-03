// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.filter.entity_class;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SFilterClassComponent final : public BaseFilterClassComponent {
		  public:
			SFilterClassComponent(ecs::BaseEntity &ent) : BaseFilterClassComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER FilterEntityClass : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
