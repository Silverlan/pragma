// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.filter.entity_name;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SFilterNameComponent final : public BaseFilterNameComponent {
		  public:
			SFilterNameComponent(ecs::BaseEntity &ent) : BaseFilterNameComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER FilterEntityName : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
