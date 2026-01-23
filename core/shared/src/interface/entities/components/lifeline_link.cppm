// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.lifeline_link;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK LifelineLinkComponent final : public BaseEntityComponent {
	  public:
		LifelineLinkComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		void RemoveEntityOnRemoval(ecs::BaseEntity *ent, Bool bRemove = true);
		void RemoveEntityOnRemoval(const EntityHandle &hEnt, Bool bRemove = true);

		virtual void InitializeLuaObject(lua::State *lua) override;
	  protected:
		std::vector<EntityHandle> m_entsRemove; // List of entities that should be removed when this component is removed
	};
};
