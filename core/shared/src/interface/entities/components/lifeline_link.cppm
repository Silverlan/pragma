// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"

export module pragma.entities.components.lifeline_link;

export namespace pragma {
	class DLLNETWORK LifelineLinkComponent final : public BaseEntityComponent {
	  public:
		LifelineLinkComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		void RemoveEntityOnRemoval(BaseEntity *ent, Bool bRemove = true);
		void RemoveEntityOnRemoval(const EntityHandle &hEnt, Bool bRemove = true);

		virtual void InitializeLuaObject(lua_State *lua) override;
	  protected:
		std::vector<EntityHandle> m_entsRemove; // List of entities that should be removed when this component is removed
	};
};
